// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "NeonDistrictCharacter.generated.h"

/**
 * 
 */

class ANeonDistrictCharacter;
class UInteractionComponent;
class UInputAction;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNeonCharacterDied, ANeonDistrictCharacter*)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingChanged, bool /*bAiming*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnReloadingChanged, bool /*bReloading*/);

UCLASS()
class CYBERPUNKPROJECT_API ANeonDistrictCharacter : public AShooterCharacter
{
	GENERATED_BODY()
	
	// 조준하면 무기 메시를 조준 위치로 보간한다
	void UpdateAimPose(float DeltaSeconds);
	
	// 조준 정도 0~1. 위치와 회전을 같은 값으로 섞어 전환 중 어긋나지 않게 한다
	float AimAlpha = 0.f;
	
protected:
	// 앞을 훑어 상호작용 대상을 찾는다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UInteractionComponent* InteractionComponent;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractAction;
	
	UPROPERTY(EditAnywhere, Category="Neon District|Aim")
	UInputAction* AimAction;
	
	// 조준했을 때 시야각. 좁을수록 확대돼 보인다
	UPROPERTY(EditAnywhere, Category="Neon District|Aim", meta = (ClampMin = 30, ClampMax = 120, Units = "Degrees"))
	float AimFOV = 55.f;
	
	// 시야각 보간 속도. 클수록 빠르다
	UPROPERTY(EditAnywhere, Category="Neon District|Aim", meta = (ClampMin = 1, ClampMax =30))
	float AimBlendSpeed = 12.f;
	
	//죽고 나서 재시작까지의 시간
	UPROPERTY(EditDefaultsOnly, Category="NeonDistrict", meta=(ClampMin=0, ClampMax=10))
	float RestartDelay = 2.f;
	
	FTimerHandle RestartTimer;
	
	UPROPERTY(VisibleInstanceOnly, Category="Neon District|Aim")
	bool bIsAiming = false;
	
	UPROPERTY(VisibleInstanceOnly, Category="Neon District|Weapon")
	bool bIsReloading = false;
	
	UPROPERTY(EditAnywhere, Category="Neon District|Weapon")
	UInputAction* ReloadAction;
	
	// 1인칭 카메라를 머리 본 기준으로 미세 조정한다.
	// 축이 본 기준이라(카메라 기본 회전이 0,90,-90 인 이유) 숫자로는 방향을 알 수 없다. 눈으로 찾는다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera")
	FVector FirstPersonCameraOffset = FVector::ZeroVector;
	
	// 1인칭 화면에서 숨길 본. 1인칭 메시가 팔이 아니라 전신이라
	// 카메라가 머리 안에 들어가 있고 머리.어깨가 화면으로 삐져나온다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Mesh")
	TArray<FName> FirstPersonHiddenBones = { TEXT("head") };
	
	// 무기를 집어넣었을 때 쓸 맨손 애니메이션. 템플릿은 무기를 내려도 애님을 되돌리지 않는다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Weapon")
	TSoftClassPtr<UAnimInstance> UnarmedAnimAsset;
	
	// BeginPlay 에서 카메라의 원래 시야각을 기억한다.
	float DefaultFOV = 0.f;
	
	// 1인칭 메시 전용 시야각의 원래 값. 월드 FOV 와 같은 비율로 움직인다
	float DefaultFirstPersonFOV = 0.f;
	
public:
	ANeonDistrictCharacter();
	
protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	//~ End AActor Interface
	
	//리스폰 대신 재시작
	virtual void Die() override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	void DoInteract();
	
	void DoStartAiming();
	void DoStopAiming();
	
	void DoReload();
	
	//레벨을 처음부터 다시 연다
	void RequestRestart();
	
	UFUNCTION(Exec)
	void NDKill();

public:
	// 죽을 때 방송된다. 미션이 구독한다.
	FOnNeonCharacterDied OnDied;
	
	//조준 상태가 바뀔 때 방송. 크로스헤어가 구독한다.
	FOnAimingChanged OnAimingChanged;
	
	bool IsAiming() const { return bIsAiming; }
	
	// 대화 중에는 총을 집어넣는다. 든 채로 의뢰를 받으면 상대에게 겨눈 그림이 된다
	void SetWeaponHolstered(bool bHolstered);
	
	// 재장전 상태가 바뀔 때 방송. 탄약 카운터가 구독한다.
	FOnReloadingChanged OnReloadingChanged;
	
	bool IsReloading() const { return bIsReloading; }
	
	// 무기가 재장전을 시작, 종료할 때 부른다
	void SetReloading(bool bNewReloading);
	
	float GetRestartDelay() const { return RestartDelay; }
};
