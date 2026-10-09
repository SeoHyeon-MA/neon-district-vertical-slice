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

	// 이동 속도에 비례해 카메라를 상하좌우로 흔든다
	void UpdateHeadBob(float DeltaSeconds);

	// 누적 위상. 멈출 때 0으로 끊으면 카메라가 튀므로 위상은 두고 진폭만 줄인다
	float HeadBobPhase = 0.f;

	// 액터 기준 오프셋. 멈출 때 중립까지 보간한다
	FVector HeadBobOffset = FVector::ZeroVector;

	// 시야에 더할 회전. 평행이동보다 이쪽이 "걸음"으로 읽힌다.
	// 카메라는 bUsePawnControlRotation 때문에 상대 회전이 매 프레임 덮어써지므로
	// 컴포넌트가 아니라 CalcCamera 결과에 더한다
	FRotator HeadBobRotation = FRotator::ZeroRotator;

	// 1인칭 팔 메시의 원래 상대 위치. 흔들림은 여기서 출발한다.
	// 카메라가 아니라 그 부모인 팔을 움직여야 팔과 시야가 함께 흔들린다
	FVector FirstPersonMeshBaseLocation = FVector::ZeroVector;

	// 점프 애니메이션이 머리 본을 밀어 올리는 것을 상쇄한다
	void UpdateHeadBoneDamping(float DeltaSeconds);

	// 머리 본 높이(액터 기준)를 느리게 따라가는 기준선.
	// 현재 높이에서 이것을 빼면 "빠른 움직임"만 남는다
	float SmoothedHeadBoneZ = 0.f;

	// 첫 프레임에는 기준선이 없다. 0에서 출발하면 그 순간 화면이 크게 튄다
	bool bHeadBoneDampInitialized = false;

	// 이번 프레임에 팔을 되돌릴 양(액터 기준 Z). UpdateHeadBob 이 흔들림과 합쳐 적용한다
	float HeadBoneDampZ = 0.f;

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

	// 걸을 때 카메라 상하 흔들림 폭(cm). 0이면 끈다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera", meta=(ClampMin=0, ClampMax=10, Units="Centimeters"))
	float HeadBobAmplitude = 2.5f;

	// 최대 속도일 때 초당 상하 왕복 횟수. 사람 걸음이 대략 2~3회다.
	// 속도가 느리면 비례해 느려진다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera", meta=(ClampMin=0.1, ClampMax=8))
	float HeadBobFrequency = 2.5f;

	// 좌우 흔들림은 상하의 몇 배인가. 주기가 절반이라 둘을 합치면 8자를 그린다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera", meta=(ClampMin=0, ClampMax=2))
	float HeadBobLateralRatio = 0.5f;

	// 조준 중 흔들림 배율. 겨누는 동안 흔들리면 조준이 안 된다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera", meta=(ClampMin=0, ClampMax=1))
	float HeadBobAimScale = 0.25f;

	// 좌우로 기우는 각도. 걸을 때 머리가 기우는 느낌을 만든다.
	// 평행이동은 먼 배경을 거의 못 움직이지만 회전은 화면 전체를 움직여 훨씬 잘 보인다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera", meta=(ClampMin=0, ClampMax=5, Units="Degrees"))
	float HeadBobRollAngle = 0.6f;

	// 위아래로 끄덕이는 각도. 발을 디딜 때의 끄덕임이다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera", meta=(ClampMin=0, ClampMax=5, Units="Degrees"))
	float HeadBobPitchAngle = 0.35f;

	// 흔들림이 목표값을 따라가는 속도. 낮추면 멈출 때 더 천천히 가라앉는다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera", meta=(ClampMin=1, ClampMax=30))
	float HeadBobBlendSpeed = 10.f;

	/**
	 *	카메라가 타고 있는 본. 점프 애니메이션이 이 본을 움직이면 시야가 통째로 튄다.
	 *	3인칭에서는 웅크렸다 뛰는 자연스러운 연출이지만 1인칭에서는 멀미가 된다
	 */
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera")
	FName CameraBoneName = TEXT("head");

	// 머리 본의 빠른 수직 움직임을 얼마나 상쇄할지. 0이면 끄고, 1이면 거의 없앤다.
	// 느린 움직임은 기준선이 따라가므로 남는다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera", meta=(ClampMin=0, ClampMax=1))
	float HeadBoneDampAmount = 0.8f;

	// 기준선이 현재 높이를 따라가는 속도. 낮출수록 더 느린 움직임까지 걸러낸다.
	// 점프는 0.23초에 16.8cm 솟구치므로 그보다 느리게 잡아야 걸린다
	UPROPERTY(EditDefaultsOnly, Category="Neon District|Camera", meta=(ClampMin=0.5, ClampMax=20))
	float HeadBoneDampSmoothing = 4.f;

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
	// 렌더링되는 시야에만 흔들림 회전을 더한다. 컨트롤 회전은 그대로라 조준은 흔들리지 않는다
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
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
