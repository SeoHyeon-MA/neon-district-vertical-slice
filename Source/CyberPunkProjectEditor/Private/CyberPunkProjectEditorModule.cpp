// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 *  에디터 전용 모듈. 지금은 켜지고 꺼질 때 할 일이 없어 엔진의 빈 구현을 그대로 쓴다.
 *  UAssetActionUtility 는 리플렉션으로 발견되므로 등록 코드가 필요 없다.
 *
 *  나중에 커스텀 에셋 타입, 디테일 패널 커스터마이징, 툴바 버튼처럼
 *  모듈이 켜질 때 등록해야 하는 것이 생기면 IModuleInterface 를 상속한 클래스를 만들고
 *  StartupModule() / ShutdownModule() 을 채운 뒤 FDefaultModuleImpl 자리에 넣는다.
 */
IMPLEMENT_MODULE(FDefaultModuleImpl, CyberPunkProjectEditor);
