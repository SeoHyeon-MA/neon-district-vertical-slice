// Fill out your copyright notice in the Description page of Project Settings.

#include "MaterialInstanceTool.h"

#include "MaterialInstanceToolSettings.h"
#include "EditorUtilityLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "MaterialEditingLibrary.h"
#include "Engine/Texture.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceConstant.h"

// LogTemp 대신 전용 카테고리를 쓰면 Output Log 에서 필터링된다
DEFINE_LOG_CATEGORY_STATIC(LogMITool, Log, All);

UMaterialInstanceTool::UMaterialInstanceTool()
{
	// 이 셋 중 하나라도 고른 상태면 우클릭 메뉴에 뜬다
	SupportedClasses.Add(UTexture::StaticClass());
	SupportedClasses.Add(UMaterialInterface::StaticClass());
	SupportedClasses.Add(UStaticMesh::StaticClass());
	SupportedClasses.Add(UMaterialInstanceToolSettings::StaticClass());
}

void UMaterialInstanceTool::CreateMaterialInstances()
{
	const TArray<UObject*> Selected = UEditorUtilityLibrary::GetSelectedAssets();

	UMaterialInstanceToolSettings* Settings = FindSettings(Selected);
	if (!Settings || !Settings->IsUsable())
	{
		UE_LOG(LogMITool, Error, TEXT("설정 에셋을 같이 선택해야 한다. 접미사 규칙이 비어 있어도 안 된다"));
		return;
	}

	UMaterialInterface* Master = FindMaster(Selected, Settings);
	if (!Master)
	{
		UE_LOG(LogMITool, Error, TEXT("부모 머티리얼이 없다. 같이 선택하거나 설정의 DefaultMaster 를 채울 것"));
		return;
	}

	TArray<FTextureSet> Sets;
	GroupTextures(Selected, Settings, Sets);

	TSet<FName> ValidParameters;
	CollectTextureParameterNames(Master, ValidParameters);

	UE_LOG(LogMITool, Log, TEXT("부모: %s / 묶음 %d 개"), *Master->GetName(), Sets.Num());

	TArray<FCreatedInstance> Created;

	for (const FTextureSet& Set : Sets)
	{
		// OutputFolder 가 비어 있으면 텍스처가 있던 자리에 만든다
		FString PackagePath = Settings->OutputFolder.Path;
		if (PackagePath.IsEmpty())
		{
			const UTexture* AnyTexture = Set.ParameterToTexture.CreateConstIterator().Value();
			PackagePath = FPackageName::GetLongPackagePath(AnyTexture->GetOutermost()->GetName());
		}

		if (UMaterialInstanceConstant* Instance = CreateInstance(Set, Master, Settings, ValidParameters, PackagePath))
		{
			Created.Add(FCreatedInstance{ Set.BaseName, Instance });
		}
	}

	UE_LOG(LogMITool, Log, TEXT("%d 개 생성. 저장은 아직 안 됐다 — Ctrl+Shift+S"), Created.Num());

	// 메쉬를 같이 골랐으면 입히는 데까지, 아니면 여기서 끝난다
	AssignToMeshes(Selected, Created, Settings);
}

UMaterialInstanceToolSettings* UMaterialInstanceTool::FindSettings(const TArray<UObject*>& Selected)
{
	for (UObject* Object : Selected)
	{
		if (UMaterialInstanceToolSettings* Settings = Cast<UMaterialInstanceToolSettings>(Object))
		{
			return Settings;
		}
	}

	return nullptr;
}

UMaterialInterface* UMaterialInstanceTool::FindMaster(const TArray<UObject*>& Selected, const UMaterialInstanceToolSettings* Settings)
{
	for (UObject* Object : Selected)
	{
		if (UMaterialInterface* Material = Cast<UMaterialInterface>(Object))
		{
			// 직접 고른 쪽이 설정의 기본값보다 우선한다
			return Material;
		}
	}

	return Settings ? Settings->DefaultMaster : nullptr;
}

void UMaterialInstanceTool::GroupTextures(const TArray<UObject*>& Selected, const UMaterialInstanceToolSettings* Settings, TArray<FTextureSet>& OutSets)
{
	// 베이스 이름 → OutSets 안의 위치
	TMap<FString, int32> BaseToIndex;

	for (UObject* Object : Selected)
	{
		UTexture* Texture = Cast<UTexture>(Object);
		if (!Texture)
		{
			continue;
		}

		const FString Name = Texture->GetName();

		// 어느 접미사로 끝나는지 찾는다
		FString MatchedSuffix;
		FName ParameterName = NAME_None;

		for (const TPair<FString, FName>& Rule : Settings->SuffixToParameter)
		{
			// 더 긴 접미사가 이긴다. "_N" 과 "_Normal" 이 같이 있을 때
			// "_N" 으로 자르면 베이스 이름이 Concrete_Normal 이 되어 묶음이 깨진다
			if (Name.EndsWith(Rule.Key) && Rule.Key.Len() > MatchedSuffix.Len())
			{
				MatchedSuffix = Rule.Key;
				ParameterName = Rule.Value;
			}
		}

		if (ParameterName.IsNone())
		{
			// 조용히 무시하면 "왜 이것만 안 들어갔지" 로 돌아온다. 이름을 남긴다
			UE_LOG(LogMITool, Warning, TEXT("접미사 규칙에 없는 텍스처라 건너뛴다: %s"), *Name);
			continue;
		}

		// T_Concrete_BC → Concrete
		FString BaseName = Name.LeftChop(MatchedSuffix.Len());
		BaseName.RemoveFromStart(Settings->TexturePrefix);

		int32* Found = BaseToIndex.Find(BaseName);
		if (!Found)
		{
			FTextureSet NewSet;
			NewSet.BaseName = BaseName;

			Found = &BaseToIndex.Add(BaseName, OutSets.Add(MoveTemp(NewSet)));
		}

		OutSets[*Found].ParameterToTexture.Add(ParameterName, Texture);
	}
}

void UMaterialInstanceTool::CollectTextureParameterNames(UMaterialInterface* Master, TSet<FName>& OutNames)
{
	TArray<FMaterialParameterInfo> Infos;
	TArray<FGuid> Ids;
	Master->GetAllTextureParameterInfo(Infos, Ids);

	for (const FMaterialParameterInfo& Info : Infos)
	{
		OutNames.Add(Info.Name);
	}
}

UMaterialInstanceConstant* UMaterialInstanceTool::CreateInstance(const FTextureSet& Set, UMaterialInterface* Master, const UMaterialInstanceToolSettings* Settings, const TSet<FName>& ValidParameters, const FString& PackagePath)
{
	const FString AssetName = Settings->InstancePrefix + Set.BaseName;

	const FString PackageName = PackagePath + TEXT("/") + AssetName;

	UMaterialInstanceConstant* Instance = nullptr;
	bool bWasExisting = false;

	// 이미 있으면 지우고 다시 만들지 않고 그 자리에서 갱신한다.
	// 한 번 돌린 뒤 메쉬에 입혀두면 그 메쉬가 참조를 쥐고 있어서, 교체하려 들면
	// "is in use" 로 막힌다. 갱신은 참조를 그대로 두므로 그 문제가 생기지 않는다
	if (FPackageName::DoesPackageExist(PackageName))
	{
		Instance = LoadObject<UMaterialInstanceConstant>(nullptr, *(PackageName + TEXT(".") + AssetName));

		if (Instance && !Settings->bUpdateExisting)
		{
			UE_LOG(LogMITool, Warning, TEXT("%s 가 이미 있어 건너뛴다. 갱신하려면 bUpdateExisting 을 켤 것"), *AssetName);
			return nullptr;
		}

		if (Instance)
		{
			Instance->Modify();
			bWasExisting = true;
		}
	}

	if (!Instance)
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
		Factory->InitialParent = Master;

		Instance = Cast<UMaterialInstanceConstant>(AssetTools.CreateAsset(
			AssetName,
			PackagePath,
			UMaterialInstanceConstant::StaticClass(),
			Factory));

		if (!Instance)
		{
			UE_LOG(LogMITool, Error, TEXT("%s 생성 실패"), *AssetName);
			return nullptr;
		}
	}

	// 팩토리의 InitialParent 는 새로 만들 때만 잡는다.
	// 갱신 경로에서는 옛 부모가 남아 있으므로 한 번 더 못박는다. 중복 호출 비용은 없다
	UMaterialEditingLibrary::SetMaterialInstanceParent(Instance, Master);

	for (const TPair<FName, UTexture*>& Pair : Set.ParameterToTexture)
	{
		// 없는 이름을 줘도 에러 없이 무시된다. MI 는 멀쩡히 만들어지고 텍스처만 비어서,
		// 원인이 "마스터 파라미터가 BaseColor 가 아니라 Base Color 였다" 같은 것이면 찾는 데 한참 걸린다
		if (!ValidParameters.Contains(Pair.Key))
		{
			UE_LOG(LogMITool, Warning, TEXT("%s: 마스터에 '%s' 텍스처 파라미터가 없다. %s 는 안 꽂힌다"),
				*AssetName, *Pair.Key.ToString(), *Pair.Value->GetName());
			continue;
		}

		UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(Instance, Pair.Key, Pair.Value);
	}

	// 셰이더 재컴파일과 썸네일 갱신. 안 부르면 썸네일이 회색으로 남아 "안 들어갔나" 싶어진다
	UMaterialEditingLibrary::UpdateMaterialInstance(Instance);

	UE_LOG(LogMITool, Log, TEXT("%s: %s/%s"), bWasExisting ? TEXT("갱신") : TEXT("만듦"), *PackagePath, *AssetName);

	return Instance;
}

void UMaterialInstanceTool::AssignToMeshes(const TArray<UObject*>& Selected, const TArray<FCreatedInstance>& Created, const UMaterialInstanceToolSettings* Settings)
{
	TArray<UStaticMesh*> Meshes;
	for (UObject* Object : Selected)
	{
		if (UStaticMesh* Mesh = Cast<UStaticMesh>(Object))
		{
			Meshes.Add(Mesh);
		}
	}

	if (Meshes.Num() == 0)
	{
		// 메쉬를 안 골랐으면 인스턴스를 만드는 데까지가 끝이다
		return;
	}

	if (Created.Num() == 0)
	{
		UE_LOG(LogMITool, Warning, TEXT("메쉬를 골랐지만 만들어진 인스턴스가 없어 입힐 것이 없다"));
		return;
	}

	for (UStaticMesh* Mesh : Meshes)
	{
		UMaterialInstanceConstant* Chosen = nullptr;

		if (Created.Num() == 1)
		{
			// 하나뿐이면 고를 것이 없다. 메쉬 이름이 달라도 그것을 입힌다
			Chosen = Created[0].Instance;
		}
		else
		{
			// 여럿이면 이름으로 짝을 찾는다. SM_Concrete_Wall ← Concrete
			// 긴 쪽이 이긴다. Concrete 와 Concrete_Wet 이 함께 있으면 후자가 더 구체적인 짝이다
			int32 BestLength = 0;
			for (const FCreatedInstance& Candidate : Created)
			{
				if (Mesh->GetName().Contains(Candidate.BaseName) && Candidate.BaseName.Len() > BestLength)
				{
					BestLength = Candidate.BaseName.Len();
					Chosen = Candidate.Instance;
				}
			}
		}

		if (!Chosen)
		{
			UE_LOG(LogMITool, Warning, TEXT("%s: 이름이 맞는 인스턴스를 못 찾아 건너뛴다"), *Mesh->GetName());
			continue;
		}

		const int32 SlotCount = Mesh->GetStaticMaterials().Num();
		if (SlotCount == 0)
		{
			UE_LOG(LogMITool, Warning, TEXT("%s: 머티리얼 슬롯이 없다"), *Mesh->GetName());
			continue;
		}

		const int32 LastSlot = Settings->bAssignToAllSlots ? SlotCount - 1 : 0;
		for (int32 i = 0; i <= LastSlot; ++i)
		{
			// 트랜잭션, PreEditChange, 슬롯 이름 보정까지 엔진이 안에서 처리한다
			Mesh->SetMaterial(i, Chosen);
		}

		UE_LOG(LogMITool, Log, TEXT("입힘: %s <- %s (슬롯 %d개)"), *Mesh->GetName(), *Chosen->GetName(), LastSlot + 1);
	}
}
