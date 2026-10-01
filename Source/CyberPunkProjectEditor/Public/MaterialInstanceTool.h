// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "MaterialInstanceTool.generated.h"

// ── 전방 선언 ──
class UTexture;
class UMaterialInterface;
class UMaterialInstanceConstant;
class UMaterialInstanceToolSettings;

/**
 *  한 묶음 — 베이스 이름 하나가 갖는 "파라미터 → 텍스처" 들.
 *  함수 사이에서만 오가는 중간 결과라 USTRUCT 으로 만들지 않는다.
 *  에디터에 노출할 일도, 저장할 일도 없다.
 */
struct FTextureSet
{
	FString BaseName;
	TMap<FName, UTexture*> ParameterToTexture;
};

/**
 *  텍스처들을 골라 우클릭하면 머티리얼 인스턴스를 만들어주는 도구.
 *
 *  텍스처 · 설정 에셋 · (선택) 마스터 머티리얼을 함께 선택한 상태에서
 *  Scripted Asset Actions 메뉴로 실행한다. 함수 인자로 받지 않고 선택 목록에서
 *  전부 찾아내는 이유는, 인자가 있으면 별도 입력 창이 떠서 동선이 길어지기 때문이다.
 *
 *  접미사와 파라미터 이름의 규칙은 설정 에셋이 들고 있고 이 클래스는 규칙을 모른다.
 */
UCLASS()
class CYBERPUNKPROJECTEDITOR_API UMaterialInstanceTool : public UAssetActionUtility
{
	GENERATED_BODY()

public:

	// ④ 생성자
	UMaterialInstanceTool();

	// ⑧ 공개 API — 우클릭 메뉴에 올라가는 항목

	UFUNCTION(CallInEditor, Category = "MITool")
	void CreateMaterialInstances();

protected:

	// ⑩ 내부 구현

	/** 선택 목록에서 설정 에셋을 집어낸다. 없으면 nullptr */
	static UMaterialInstanceToolSettings* FindSettings(const TArray<UObject*>& Selected);

	/** 선택 목록의 마스터가 우선, 없으면 설정의 DefaultMaster */
	static UMaterialInterface* FindMaster(const TArray<UObject*>& Selected, const UMaterialInstanceToolSettings* Settings);

	/** 텍스처들을 베이스 이름으로 묶는다. 접미사가 규칙에 없으면 건너뛴다 */
	static void GroupTextures(const TArray<UObject*>& Selected, const UMaterialInstanceToolSettings* Settings, TArray<FTextureSet>& OutSets);

	/** 마스터가 가진 텍스처 파라미터 이름들을 모은다. 없는 이름에 꽂으면 조용히 무시되므로 미리 거른다 */
	static void CollectTextureParameterNames(UMaterialInterface* Master, TSet<FName>& OutNames);

	/** 묶음 하나로 머티리얼 인스턴스 에셋을 만든다. 실패하면 nullptr */
	static UMaterialInstanceConstant* CreateInstance(const FTextureSet& Set, UMaterialInterface* Master, const UMaterialInstanceToolSettings* Settings, const TSet<FName>& ValidParameters, const FString& PackagePath);
};
