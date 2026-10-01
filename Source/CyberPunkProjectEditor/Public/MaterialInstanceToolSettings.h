// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MaterialInstanceToolSettings.generated.h"

class UMaterialInterface;

/**
 *  텍스처 이름에서 머티리얼 인스턴스를 만들어내는 도구의 규칙.
 *
 *  규칙을 코드가 아니라 여기에 두는 이유는, 외부에서 받은 에셋마다
 *  접미사 관례가 다르고(_BC / _D / _Albedo) 마스터의 파라미터 이름도 작업 중에 바뀌기 때문이다.
 *  코드에 박으면 그때마다 빌드하고 에디터를 다시 띄워야 한다.
 *
 *  에디터 전용이다. 게임 빌드에는 존재하지 않는다.
 */
UCLASS(BlueprintType)
class CYBERPUNKPROJECTEDITOR_API UMaterialInstanceToolSettings : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// -- 부모 --
	
	/** 만들어질 인스턴스의 기본 부모. 콘텐츠 브라우저에서 메테리얼을 같이 선택하면 그쪽이 우선한다 */
	UPROPERTY(EditAnywhere, Category="MITool|Parent")
	TObjectPtr<UMaterialInterface> DefaultMaster = nullptr;
	
	// -- 이름 규칙 --
	
	/**
	 *  텍스처 이름 끝의 접미사 → 마스터의 텍스처 파라미터 이름.
	 *  예: "_BC" → "BaseColor", "_N" → "Normal", "_ORM" → "ORM"
	 *  키에 밑줄을 포함해 적는다. 베이스 이름을 잘라낼 기준이기도 하다.
	 */
	UPROPERTY(EditAnywhere, Category = "MITool|Naming")
	TMap<FString, FName> SuffixToParameter;

	/** 텍스처 이름 앞에 붙은 접두사. 베이스 이름을 뽑을 때 떼어낸다 */
	UPROPERTY(EditAnywhere, Category = "MITool|Naming")
	FString TexturePrefix = TEXT("T_");

	/** 만들어질 인스턴스 이름 앞에 붙일 접두사 */
	UPROPERTY(EditAnywhere, Category = "MITool|Naming")
	FString InstancePrefix = TEXT("MI_");

	// ── 출력 ──

	/** 비워두면 텍스처가 있던 폴더에 만든다. 채우면 그 경로에 모아 만든다 */
	UPROPERTY(EditAnywhere, Category = "MITool|Output", meta = (ContentDir))
	FDirectoryPath OutputFolder;

	/**
	 *  같은 이름의 인스턴스가 이미 있을 때 그 자리에서 갱신할지. 꺼두면 건너뛰고 경고만 남긴다.
	 *  지우고 다시 만들지 않는 이유는, 이미 메쉬에 입혀둔 인스턴스는 그 메쉬가 참조를 쥐고 있어
	 *  교체하려 들면 "is in use" 로 막히기 때문이다. 갱신은 참조를 그대로 둔다.
	 */
	UPROPERTY(EditAnywhere, Category = "MITool|Output")
	bool bUpdateExisting = false;

	// ── 메쉬에 입히기 ──

	/**
	 *  스태틱 메쉬를 같이 선택했을 때, 만든 인스턴스를 모든 머티리얼 슬롯에 넣을지.
	 *  끄면 0번 슬롯만 바꾼다. 슬롯이 여럿인 메쉬를 한 머티리얼로 통째로 덮고 싶지 않을 때 쓴다.
	 */
	UPROPERTY(EditAnywhere, Category = "MITool|Assign")
	bool bAssignToAllSlots = true;

public:

	// ── 공개 API ──

	/** 이 설정으로 실제로 일을 할 수 있는 상태인지 */
	bool IsUsable() const { return SuffixToParameter.Num() > 0; }
};
