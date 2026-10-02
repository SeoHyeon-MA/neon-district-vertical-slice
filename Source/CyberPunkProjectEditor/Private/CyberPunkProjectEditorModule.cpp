// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "ContentBrowserMenuContexts.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Texture.h"
#include "MaterialInstanceTool.h"

#define LOCTEXT_NAMESPACE "CyberPunkProjectEditor"

DEFINE_LOG_CATEGORY_STATIC(LogCyberPunkEditor, Log, All);

/**
 *  에디터 전용 모듈. 콘텐츠 브라우저 우클릭 메뉴를 직접 등록한다.
 *
 *  Blutility 의 Scripted Asset Actions 를 쓰지 않는 이유는, 그쪽이 에셋 레지스트리에서
 *  UEditorUtilityBlueprint 에셋만 훑기 때문에 네이티브 C++ 클래스가 올라갈 수 없고,
 *  껍데기 블루프린트를 하나 만들어 두어야 하기 때문이다.
 */
class FCyberPunkProjectEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override
	{
		// 모듈 로드 시점에는 툴메뉴가 아직 없을 수 있다. 준비되면 불러달라고 등록한다
		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCyberPunkProjectEditorModule::RegisterMenus));
	}

	virtual void ShutdownModule() override
	{
		UToolMenus::UnRegisterStartupCallback(this);

		// 소유자 단위로 등록해뒀으므로 한 번에 걷힌다.
		// 이게 없으면 Live Coding 으로 모듈을 다시 로드할 때마다 메뉴 항목이 중복으로 쌓인다
		UToolMenus::UnregisterOwner(this);
	}

private:

	void RegisterMenus()
	{
		// 이 범위 안에서 등록한 것은 전부 this 소유가 된다. ShutdownModule 의 UnregisterOwner 와 짝이다
		FToolMenuOwnerScoped OwnerScoped(this);

		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("ContentBrowser.AssetContextMenu"));
		if (!Menu)
		{
			return;
		}

		FToolMenuSection& Section = Menu->FindOrAddSection(
			TEXT("NeonDistrictTools"),
			LOCTEXT("SectionLabel", "Neon District"));

		// 동적 엔트리여야 메뉴가 열리는 시점의 선택 상태를 읽을 수 있다.
		// 정적으로 등록하면 선택이 무엇인지 모르는 채로 항목만 만들어진다
		Section.AddDynamicEntry(TEXT("CreateMaterialInstances"), FNewToolMenuSectionDelegate::CreateLambda(
			[](FToolMenuSection& InSection)
			{
				UContentBrowserAssetContextMenuContext* Context =
					InSection.FindContext<UContentBrowserAssetContextMenuContext>();

				if (!Context)
				{
					return;
				}

				// 선택에 텍스처가 하나도 없으면 항목을 만들지 않는다. 이 도구가 하는 일이
				// "텍스처에서 인스턴스를 만든다" 라서, 텍스처가 없으면 할 일 자체가 없다.
				// 설정 에셋·마스터·메쉬는 텍스처와 함께 골랐을 때 의미가 생기는 보조 선택이다.
				//
				// FAssetData 만 보고 판단한다 — 여기서 로드하면 폴더를 통째로 고른 우클릭 한 번에
				// 수백 장을 읽게 된다. IsInstanceOf 는 기본값이 EResolveClass::No 라 클래스를 로드하지 않고,
				// 상속도 따지므로 UTexture2D·UTextureCube 가 모두 걸린다
				const bool bHasTexture = Context->SelectedAssets.ContainsByPredicate(
					[](const FAssetData& Asset)
					{
						return Asset.IsInstanceOf<UTexture>();
					});

				if (!bHasTexture)
				{
					return;
				}

				InSection.AddMenuEntry(
					TEXT("CreateMaterialInstances"),
					LOCTEXT("CreateMILabel", "Create Material Instances"),
					LOCTEXT("CreateMITooltip", "텍스처 이름 규칙에 따라 머티리얼 인스턴스를 만들고, 메쉬를 함께 골랐으면 입힌다"),
					FSlateIcon(),
					FExecuteAction::CreateLambda([Context]()
					{
						// UE 5.5 부터 우클릭만으로 에셋을 자동 로드하지 않는다.
						// 텍스처 내용을 읽어야 하므로 실행 시점에 명시적으로 로드한다
						const TArray<UObject*> Selected = Context->LoadSelectedObjects<UObject>();

						MaterialInstanceTool::CreateMaterialInstances(Selected);
					}));
			}));
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCyberPunkProjectEditorModule, CyberPunkProjectEditor);
