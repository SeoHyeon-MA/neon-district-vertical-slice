// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *  텍스처 이름 규칙에서 머티리얼 인스턴스를 만든다.
 *  선택 목록에 메쉬가 있으면 입히는 데까지, 없으면 만드는 데까지 한다.
 *
 *  상태가 없으므로 UCLASS 가 아니라 자유 함수다. 우클릭 메뉴는 모듈이 UToolMenus 로
 *  직접 등록하므로 리플렉션도 필요 없다. LevelArt 의 PropMeshBuilder 와 같은 모양이다.
 *
 *  밖에서 부르는 것은 이 하나뿐이고, 묶기·생성·입히기 헬퍼와 중간 구조체는 전부 .cpp 안에 있다.
 */
namespace MaterialInstanceTool
{
	/** 설정 에셋 · 마스터 · 텍스처 · (선택) 스태틱 메쉬가 섞인 선택 목록을 그대로 받는다 */
	void CreateMaterialInstances(const TArray<UObject*>& Selected);
}
