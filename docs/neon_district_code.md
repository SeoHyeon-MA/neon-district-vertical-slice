# Neon District — C++ 코드 정리

이 프로젝트 고유의 게임플레이 C++ 코드(`Source/CyberPunkProject/*/NeonDistrict/`)가
무엇을 하고, 왜 그렇게 짰는지 정리한 문서다.

기준 시점: `c27b387 feat: split the mission into an abstract base and a warehouse subclass` (2026-09-16).
새 클래스를 만들 때 헤더 안의 배치 순서는 9절 코딩 규약을 따른다.
4절은 초기 작성 당시(2026-09-08)의 변경 기록이라 그대로 두었다. 최신 흐름은 7절을 본다.

---

## 1. 전체 그림

이 프로젝트의 게임플레이는 UE5 **Shooter 템플릿**(`Variant_Shooter/`)에서 출발한다.
템플릿 원본은 **수정하지 않고 참고 자료로 남긴다**는 것이 `docs/roadmap.md` 1번 항목의 방침이고,
NeonDistrict 코드는 그 템플릿을 **상속으로 덮어쓰는 층**이다.

```text
AGameModeBase
 └─ AShooterGameMode        (템플릿, UCLASS(abstract) — 직접 쓸 수 없다)
     └─ ANeonDistrictGameMode   ← 폰·컨트롤러·UI 지정, 시작 무기, 체크포인트 재시작

ACharacter
 └─ ACyberPunkProjectCharacter (템플릿 — 1인칭 카메라, 이동)
     └─ AShooterCharacter       (템플릿 — 사격, 체력, 사망)
         └─ ANeonDistrictCharacter  ← 리스폰 대신 재시작 요청, OnDied 방송, 상호작용
             └─ BP_NeonDistrictCharacter (BP_ShooterCharacter 복제 후 부모 변경 — 메시·애님·입력)

AActor
 └─ AShooterWeapon          (템플릿, UCLASS(abstract) — 발사/재장전/탄약 로직)
     └─ ANeonDistrictWeapon     (abstract — 스태틱 메시 총 + 소프트 참조 + BeginPlay 로드)
         ├─ ANeonDistrictPistol     ← 한 손 애님, 단발
         └─ ANeonDistrictRifle      ← 두 손 애님, 연사 30발

AActor
 └─ ANeonDistrictMission    ← 진입 트리거, 상태 머신, 사망 카운트, 재시도 루프
```

| 파일 | 역할 |
|---|---|
| `NeonDistrictGameMode.h/.cpp` | 폰·컨트롤러·UI 클래스 지정(`InitGame`), 스폰마다 무기 지급(`SetPlayerDefaults`), 체크포인트 재시작(`RestartPlayer`), `OnPlayerPawnReady` 방송 |
| `NeonDistrictCharacter.h/.cpp` | `Die()` 오버라이드 — 템플릿 리스폰 취소, 지연 후 재시작 요청, `OnDied` 방송. 상호작용 컴포넌트·E키 바인딩·프롬프트 위젯 생성. 콘솔용 `NDKill` |
| `NeonDistrictWeapon.h/.cpp` | 스태틱 메시 총 컴포넌트, 에셋 소프트 참조, `BeginPlay`에서 로드·오프셋 적용 |
| `NeonDistrictPistol.h/.cpp`, `NeonDistrictRifle.h/.cpp` | 생성자에서 경로·오프셋·탄창·연사만 채움 |
| `NeonDistrictMission.h/.cpp` | 진입 트리거, `EMissionState`, `DeathCount`, 델리게이트 구독, `SetupSegment` 루프. 콘솔용 `ND.AcceptMission` |
| `Interactable.h` | `IInteractable` — 프롬프트 문구·가능 여부·실행. 문·키·창고·NPC가 구현 |
| `InteractionComponent.h/.cpp` | 시선 앞 탐색, 대상 변경 방송, E키 처리. 캐릭터에 부착 |
| `InteractionPromptWidget.h/.cpp` | 대상 변경 구독, 문구 표시·숨김. UMG는 `WBP_InteractionPrompt` |
| `InteractableTest.h/.cpp` | 배선 검증용 최소 구현체 |

헤더는 `Public/NeonDistrict/`, 구현은 `Private/NeonDistrict/`에 있다.

---

## 2. `ANeonDistrictGameMode`

### 목적

`AShooterGameMode`는 `UCLASS(abstract)`라 레벨에 직접 지정할 수 없다.
그래서 **구체 클래스가 하나 필요했고**, 동시에 로드맵 1번의 "시작 무기 지급"을 여기에 붙였다.
지금 이 클래스가 하는 일은 세 가지다.

1. 어떤 폰·컨트롤러·UI를 쓸지 정한다 (`InitGame`)
2. 폰이 스폰될 때마다 총을 쥐어주고, 끝나면 `OnPlayerPawnReady`를 방송한다 (`SetPlayerDefaults`)
3. 체크포인트가 있으면 거기서 재시작시킨다 (`RestartPlayer`, `SetCheckpoint`)

미션에 대해서는 **아무것도 모른다.** 사실을 방송할 뿐 판단하지 않는다 (7절).

### 멤버

```cpp
UPROPERTY(EditDefaultsOnly, Category="Neon District")
TArray<TSubclassOf<AShooterWeapon>> StartingWeapons;
```

- 생성자에서 `ANeonDistrictPistol`, `ANeonDistrictRifle`을 순서대로 넣는다. 나중에 넣은 것을 들고 시작한다.
- `TSubclassOf<>`라서 나중에 다른 무기로 바꿀 때 코드를 고칠 필요가 없다.
- `EditDefaultsOnly`이므로 인스턴스가 아니라 클래스 기본값(BP 파생 시)에서만 바꾼다.
- 전방 선언 `class AShooterWeapon;`만 두고 헤더에서 `ShooterWeapon.h`를 include 하지 않았다 — 헤더 의존을 줄이는 정석 패턴.

### `InitGame()` — 왜 생성자가 아닌가

```cpp
DefaultPawnClass       ← /Game/Variant_Shooter/Blueprints/BP_ShooterCharacter
PlayerControllerClass  ← /Game/Variant_Shooter/Blueprints/BP_ShooterPlayerController
ShooterUIClass         ← /Game/Variant_Shooter/UI/UI_Shooter
```

세 값 모두 **블루프린트 에셋**을 가리킨다. 메시·애니메이션·Enhanced Input 매핑·카메라 세팅이
그 BP 안에 들어 있어서, C++ 클래스(`AShooterCharacter` 등)를 그대로 지정하면 맨몸에 입력도 없는 폰이 나온다.

BP를 잡는 방법은 두 가지인데 여기서는 뒤쪽을 골랐다.

| 방법 | 시점 | 성격 |
|---|---|---|
| `ConstructorHelpers::FClassFinder` | CDO 생성 시(에디터 기동 포함) | 경로가 틀리면 **에디터가 뜨는 순간 크래시** |
| `LoadClass<T>(...)` in `InitGame` | 게임 시작 직전 | 실패해도 `if`로 걸러져 조용히 넘어감 |

`InitGame`은 게임모드 초기화 중 **폰이 스폰되기 전에** 불리므로,
여기서 `DefaultPawnClass`를 바꿔도 늦지 않는다. 그래서 안전한 런타임 로드를 쓸 수 있었다.

### `SetPlayerDefaults()` — 왜 다음 틱인가

처음에는 `HandleStartingNewPlayer_Implementation`에 있었다. 그 함수는 **첫 접속에만** 불려서
부활한 플레이어가 맨손이 되는 문제가 있었고, 스폰마다 불리는 `SetPlayerDefaults`로 옮겼다.

```cpp
Super::SetPlayerDefaults(PlayerPawn);

TWeakObjectPtr<APawn> WeakPawn(PlayerPawn);
GetWorldTimerManager().SetTimerForNextTick([this, WeakPawn]()
{
    if (!WeakPawn.IsValid()) return;
    if (IShooterWeaponHolder* Holder = Cast<IShooterWeaponHolder>(WeakPawn.Get()))
    {
        for (const TSubclassOf<AShooterWeapon>& WeaponClass : StartingWeapons)
        {
            if (WeaponClass) Holder->AddWeaponClass(WeaponClass);
        }
        OnPlayerPawnReady.Broadcast(WeakPawn.Get());   // 미션이 이걸 듣고 새 폰에 재구독한다
    }
});
```

핵심 세 가지.

- **다음 틱으로 미룸** — 빙의 직후 프레임에는 폰의 메시/애님 초기화가 아직 진행 중이다.
  그 시점에 무기를 붙이면 `AttachWeaponMeshes`가 소켓을 못 찾거나 애님 인스턴스 교체가 씹힐 수 있다.
  한 틱 뒤로 미뤄 초기화가 끝난 상태를 보장한다.
- **`TWeakObjectPtr`** — 람다가 다음 틱에 실행되는 사이에 폰이 파괴될 수 있다.
  raw 포인터를 캡처하면 댕글링이 되므로 약참조로 잡고 `IsValid()`로 확인한다.

지급 경로는 **`IShooterWeaponHolder` 인터페이스**를 통한다.
`AShooterCharacter`를 직접 캐스팅하지 않으므로, 나중에 플레이어 클래스를 갈아끼워도
그 클래스가 인터페이스만 구현하면 이 코드는 그대로 동작한다.

`AddWeaponClass`는 내부에서 무기 액터를 스폰하고 → 기존 무기를 `DeactivateWeapon` → 새 무기를 `ActivateWeapon` 한다.
`ActivateWeapon`이 `OnWeaponActivated`를 부르고, 거기서 캐릭터의 1인칭/3인칭 메시 애님 인스턴스가 무기의 것으로 교체된다.

---

## 3. `ANeonDistrictWeapon`

### 목적

발사·재장전·탄약·리코일·소음 감지는 전부 부모 `AShooterWeapon`이 이미 갖고 있다.
이 클래스가 추가하는 것은 **"이 무기는 어떤 외형과 어떤 에셋을 쓰는가"** 뿐이다.
즉 템플릿에서라면 블루프린트로 만들었을 무기 변형을 C++로 만든 것이다.

### 생성자 — 컴포넌트 구성

```cpp
GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
GunMesh->SetupAttachment(GetFirstPersonMesh());
GunMesh->SetCollisionProfileName(FName("NoCollision"));
GunMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
GunMesh->bOnlyOwnerSee = true;
```

| 줄 | 이유 |
|---|---|
| `CreateDefaultSubobject` | 컴포넌트 생성은 **반드시 생성자에서만**. UE의 서브오브젝트 규칙 |
| `SetupAttachment(GetFirstPersonMesh())` | 부모가 만든 1인칭 스켈레탈 메시의 자식. 부모 메시가 캐릭터 손 소켓에 붙으면 총도 따라간다 |
| `NoCollision` | 자기가 쏜 총알이 자기 총 모델에 맞는 것을 막는다 |
| `FirstPerson` 프리미티브 타입 | UE5.5+ 1인칭 전용 렌더링 패스. 벽 뚫림·FOV 왜곡 처리를 엔진이 담당 |
| `bOnlyOwnerSee` | 다른 플레이어 화면에는 이 1인칭 모델이 보이지 않는다 |

`UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))` —
private 멤버를 디테일 패널과 BP에서 **읽기만** 가능하게 노출하는 표준 조합이다.
`VisibleAnywhere`라서 에디터에서 교체는 못 하고, 값 확인만 된다.

### `BeginPlay()` — 에셋 바인딩

```cpp
GunMesh                     ← /Game/Fab/.../sci_fi_gunvenra_46      (스태틱 메시)
ProjectileClass             ← BP_ShooterProjectile_Bullet           (투사체)
FirstPersonAnimInstanceClass ← ABP_FP_Pistol                        (팔 애님)
ThirdPersonAnimInstanceClass ← ABP_TP_Pistol                        (몸 애님)

Super::BeginPlay();   // ← 마지막에 호출
```

`Super::BeginPlay()`를 **함수 끝에서** 부르는 것이 의도적이다.
부모의 `BeginPlay`가 `AttachWeaponMeshes`를 호출하고, 그 직후 `AddWeaponClass`가 `ActivateWeapon` →
`OnWeaponActivated`로 이어지면서 애님 인스턴스 클래스를 읽어간다.
읽히기 전에 값이 채워져 있어야 하므로 로드를 먼저 끝낸다.

메시 트랜스폼(`Scale 0.3`, `Location 0`, `Rotation 0`)은 메시가 성공적으로 로드된 `if` 블록 안에서 조정한다.
지금은 위치·회전이 0이라 손 소켓에 그대로 스냅되며, 총구 정렬이 어긋나면 여기 숫자를 조정하는 지점이다.

---

## 4. 이번 작업에서 바뀐 것

`4556b5b` 커밋 대비 변경 내역이다.

### 게임모드 — 사실상 신규 구현

이전에는 헤더에 빈 `UCLASS`만 있고 `.cpp`는 include 한 줄뿐이었다.
`StartingWeaponClass`, 생성자, `InitGame`, `HandleStartingNewPlayer_Implementation`이 전부 이번에 추가됐다.

### 무기 — 에셋 로드를 생성자에서 `BeginPlay`로 이동

가장 중요한 구조 변경이다.

**이전 (생성자 + `ConstructorHelpers`)**

```cpp
static ConstructorHelpers::FObjectFinder<UStaticMesh> GunMeshAsset(TEXT("/Game/Fab/..."));
if (GunMeshAsset.Succeeded()) { GunMesh->SetStaticMesh(GunMeshAsset.Object); }

static ConstructorHelpers::FClassFinder<AShooterProjectile> ProjectileBP(TEXT("/Game/..."));
if (ProjectileBP.Succeeded()) { ProjectileClass = ProjectileBP.Class; }
```

**이후 (`BeginPlay` + `LoadObject` / `LoadClass`)**

```cpp
if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Fab/..."))) { ... }
if (UClass* Projectile = LoadClass<AShooterProjectile>(nullptr, TEXT("/Game/..."))) { ... }
```

바꾼 이유:

- `ConstructorHelpers`는 **생성자에서만** 동작하고, CDO 생성 시점에 경로를 해석한다.
  경로가 틀리면 에디터 기동 중 어서션으로 죽는다. `LoadObject`는 null을 반환할 뿐이라 `if`로 방어된다.
- `FClassFinder`는 경로 끝의 `_C`를 자동으로 붙이는 등 규칙이 달라 헷갈린다.
  `LoadClass`는 `..._C`를 명시적으로 쓰므로 경로가 눈에 보인다.
- 애님 BP 두 개(`ABP_FP_Pistol`, `ABP_TP_Pistol`)를 새로 바인딩했다. 이전 버전에는 없었다.
  이게 없으면 총은 들리는데 팔 애니메이션이 권총 포즈로 바뀌지 않는다.

**트레이드오프** — `LoadObject`/`LoadClass`는 **동기 로드**다.
무기가 처음 스폰되는 순간 에셋이 메모리에 없으면 그 프레임에 히치가 생긴다.
지금은 무기가 게임 시작 시 한 번만 스폰되므로 체감되지 않지만,
전투 중에 무기를 자주 스폰하게 되면 `TSoftObjectPtr` + 비동기 로드로 옮기는 것이 정석이다.

### 레벨 — GameMode Override 지정

`Content/NeonDistrict/Maps/Lvl_NeonDistrict.umap`이 이번 변경에서 `NeonDistrictGameMode`를 참조하게 됐다.
커밋된 버전에는 그 참조가 없다. 즉 **World Settings의 GameMode Override**를 이번에 지정한 것이다.

이게 필요한 이유: `Config/DefaultEngine.ini`의 `GlobalDefaultGameMode`는 **여전히 FirstPerson 템플릿**을 가리킨다.

```ini
GlobalDefaultGameMode=/Game/FirstPerson/Blueprints/BP_FirstPersonGameMode.BP_FirstPersonGameMode_C
```

레벨 오버라이드가 이 전역 설정을 이기므로 `Lvl_NeonDistrict`에서는 NeonDistrict 게임모드가 뜬다.
다른 맵을 만들면 다시 FirstPerson 템플릿이 뜬다는 뜻이기도 하다.

### NeonDistrict와 무관한 변경

작업 트리에 같이 올라와 있지만 이 작업과 관계없는 것들이다.

- `.mcp.json` — Rider MCP 서버 항목 추가, 들여쓰기 재포맷
- `Config/DefaultEditor.ini` — `AdvancedPreviewScene.SharedProfiles` 섹션(에디터 프리뷰 씬 설정) 자동 추가

---

## 5. 로드맵 1번 대비 진행 상황

`docs/roadmap.md`의 "1. 전용 GameMode 신설과 아레나 규칙 제거" 체크리스트 기준이다.

| 항목 | 상태 |
|---|---|
| `Source/CyberPunkProject/NeonDistrict/` 폴더 신설 | 완료 (Public/Private 분리) |
| `ANeonDistrictGameMode` 생성 | 완료 |
| `ANeonDistrictCharacter` 생성 | 완료 — `BP_NeonDistrictCharacter`가 이 클래스를 부모로 쓴다 |
| 리스폰 비활성화 / 체크포인트 재시작 | 완료 — 템플릿 타이머 취소 후 `RestartPlayer`가 체크포인트로 보낸다 (7절) |
| 시작 무기 지급 (`AddWeaponClass`) | 완료 |
| `Lvl_NeonDistrict`에 GameMode Override | 완료 |
| `PlayerStart` (-6600, 0, 120) 확인 | 미확인 |

---

## 6. 정리해두면 좋을 자잘한 것

기능에는 영향이 없지만 다음에 손댈 때 같이 치울 것들이다.

- `NeonDistrictGameMode.cpp`의 `#include "UObject/ConstructorHelpers.h"` — 이제 `ConstructorHelpers`를 쓰지 않으므로 불필요하다.
- 같은 파일 `ShooterUI.h` include 옆 주석이 `// ← FClassFinder<APawn>`으로 잘못 붙어 있다. 실제로는 `ShooterUIClass` 타입 때문에 필요한 include다.
- `NeonDistrictGameMode.h`에서 `InitGame` 위에 `HandleStartingNewPlayer`용 주석("플레이어가 스폰되어 폰에 빙의한 직후 호출된다")이 중복으로 붙어 있다.
- `NeonDistrictWeapon.cpp`의 `#include "Components/SkeletalMeshComponent.h"` — 직접 쓰는 곳이 없다(`GetFirstPersonMesh()`의 반환 타입 때문인데 헤더 체인으로 이미 들어온다).
- `GunMesh`의 `SetRelativeLocation(0)` / `SetRelativeRotation(0)`은 기본값과 같아 실질적인 동작이 없다. 총구 정렬을 잡을 때 쓸 자리 표시로 남겨둔 것이라면 그대로 두어도 된다.

---

## 7. 사망 → 미션 재시도 — 델리게이트로 잇기

로드맵 1번의 리스폰 항목과 기획서의 "죽으면 미션 재시도, 사망 횟수로 랭크"를 구현한 부분이다.
관련 커밋: `b2a77e4`, `bb1e60f`, `d302fec`, `eea6986`, `ece1d88`.

### 7-1. 왜 델리게이트인가

처음에는 게임모드가 미션 포인터(`ActiveMission`)를 들고, 죽을 때마다 직접 물어봤다.

```cpp
// 이전 — 게임모드가 미션 사정을 알아야 했다
if (ActiveMission.IsValid()) { ActiveMission->OnPlayerDied(); }
```

게임모드는 규칙을 담당하는 클래스인데 미션 내부까지 알아야 했고, 미션은 누가 불러주기만 기다렸다.
지금은 **방송과 구독**으로 뒤집었다. 방송하는 쪽은 누가 듣는지 모르고, 미션이 시작할 때 스스로 구독한다.
게임모드는 미션이 존재하는지조차 모른다.

| 역할 | 하는 일 | 상대를 아는가 |
|---|---|---|
| 방송 | 일이 생기면 `Broadcast()` | 모름 |
| 구독 | 미리 `AddUObject()`로 등록 | 방송자를 알고 등록 |

템플릿도 같은 방식을 쓴다 — `AShooterNPC::OnPawnDeath`를 스포너와 AI 컨트롤러가 각자 구독한다.

### 7-2. 세 파일의 역할

**캐릭터** — 죽음을 방송한다.

```cpp
DECLARE_MULTICAST_DELEGATE_OneParam(FOnNeonCharacterDied, ANeonDistrictCharacter*);
FOnNeonCharacterDied OnDied;
// Die() 안, Super::Die() 직후
OnDied.Broadcast(this);
```

**게임모드** — 새 폰이 무기까지 받은 뒤 방송한다.

```cpp
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerPawnReady, APawn*);
FOnPlayerPawnReady OnPlayerPawnReady;
// SetPlayerDefaults 람다 끝
OnPlayerPawnReady.Broadcast(WeakPawn.Get());
```

**미션** — 둘 다 구독하고 자기 루프를 돈다.

```cpp
GameMode->OnPlayerPawnReady.AddUObject(this, &ANeonDistrictMission::BindToPlayer);
Character->OnDied.AddUObject(this, &ANeonDistrictMission::HandlePlayerDied);
```

### 7-3. 흐름

```text
ND.AcceptMission                    State: NotAccepted → Accepted
    │
트리거 진입 → StartMission()        State: → InProgress, DeathCount = 0
    ├ SetCheckpoint(RestartPoint)
    ├ OnPlayerPawnReady 구독         ← 앞으로 새 폰마다 BindToPlayer
    ├ BindToPlayer(현재 폰)          ← OnDied 구독
    └ SetupSegment()
    │
사망 → ANeonDistrictCharacter::Die()
    ├ Super::Die()                   무기 끄기, 이동 정지, 입력 차단
    ├ OnDied.Broadcast
    │    └ HandlePlayerDied()        DeathCount = 1
    │         └ SetupSegment()       ← "처음부터 다시"
    ├ 템플릿 RespawnTimer 취소
    └ RestartDelay 뒤 RequestRestart()
         컨트롤러 확보 → Destroy() → GameMode->RestartPlayer(컨트롤러)
    │
RestartPlayer → 체크포인트에서 스폰
    └ SetPlayerDefaults → 무기 지급
         └ OnPlayerPawnReady.Broadcast
              └ BindToPlayer(새 폰)   ← OnDied 재구독
    │
사망 → ... DeathCount = 2
```

### 7-4. 재구독이 필요한 이유

죽으면 폰이 파괴되고, 그 폰의 `OnDied`에 걸어둔 구독도 함께 사라진다.
새 폰은 아무도 구독하지 않은 델리게이트를 갖고 태어나므로,
게임모드의 `OnPlayerPawnReady`를 받아 **새 폰에 다시 구독**해야 한다.
이게 없으면 첫 죽음만 세고 두 번째부터는 조용해진다. 검증 기준은 `사망 2회`가 찍히는지다.

### 7-5. 폰을 파괴해도 엔진은 부활시키지 않는다

엔진이 `RestartPlayer`를 자동으로 부르는 경로는 `APlayerController::StartFire`(`PlayerController.cpp:3262`) 하나뿐이고,
이건 옛 입력 시스템 경로라 Enhanced Input에서는 도달하지 않는다.
템플릿 주석의 *"destroy the character to force the PC to respawn"* 은 이 프로젝트에서 성립하지 않는다.
그래서 `RequestRestart()`가 컨트롤러를 미리 잡아두고 `Destroy()` 뒤 직접 `RestartPlayer`를 부른다.

### 7-6. 미션 상태 — 열거형

```cpp
enum class EMissionState : uint8
{
    NotAccepted,   // NPC에게 아직 안 받음
    Accepted,      // 받았지만 구역 진입 전
    InProgress,    // 진입 후 ~ 완료 전. 죽어도 여기 머문다
    Completed
};
```

`bAccepted`, `bRunning`, `AttemptCount` 세 개를 하나로 합쳤다.
`bool` 여러 개는 `bAccepted=false`인데 `bRunning=true` 같은 있을 수 없는 조합이 만들어질 수 있지만,
열거형은 한 번에 한 값만 가져서 그런 상태 자체가 불가능하다.

`InProgress`에 머무는 동안 트리거를 드나들어도 무시된다. 미션은 **한 번 시작되면 완료까지 진행 중**이다.
사망은 `DeathCount`로만 센다. 미션 밖에서 죽으면 `State != InProgress`라 세지 않는다 — 랭크가 미션 안의 죽음만 반영하는 이유다.

### 7-7. `SetupSegment()` — 처음부터 다시 도는 함수

```text
StartMission()      → DeathCount = 0 → SetupSegment()
HandlePlayerDied()  → DeathCount++   → SetupSegment()
```

시작할 때와 죽은 뒤가 **같은 함수**를 부른다. 지금은 로그만 찍지만, 적 정리·생성이 여기 들어가면
"죽으면 구간이 처음 상태로 돌아간다"가 코드 한 곳으로 보장된다. 별도의 재시도 경로는 없다.

### 7-8. 트리거 볼륨 설정에서 걸린 것

- **`WorldStatic`으로 두면 손이 꺾인다.** 1인칭 손 리그(`Ctrl_HandAdjusment_*`)가 총이 벽을 뚫지 않도록 지형을 훑는데,
  트리거가 지형으로 분류돼 있으면 보이지 않는 벽으로 인식한다. `WorldDynamic`으로 분류만 바꾸면 해결된다.
  응답 목록(`Pawn`만 `Overlap`)은 문제가 아니었고, 그대로 두어 충돌 단계에서 필터링한다.
- **`RestartPoint`는 트리거 바깥.** 트리거 안에서 부활하면 그 자리에서 다시 시작되어 "건물에 진입하세요" 흐름이 성립하지 않는다.
  `UArrowComponent`를 별도로 두고 에디터에서 문 밖으로 옮긴다.
- **`UFUNCTION(Exec)`는 레벨 액터에서 안 된다.** 콘솔은 PlayerController → Pawn → HUD → GameMode → ... 정해진 목록만 뒤진다(`Player.cpp:122`).
  미션 수락 디버그 명령은 `FAutoConsoleCommandWithWorld`로 `ND.AcceptMission`을 등록했다. `#if !UE_BUILD_SHIPPING`으로 감싼다.

### 7-9. 이번에 쓴 API

| | 뜻 |
|---|---|
| `DECLARE_MULTICAST_DELEGATE_OneParam(이름, 인자타입)` | 인자 하나 받는 델리게이트 타입. 구독자 여럿 가능 |
| `.AddUObject(this, &클래스::함수)` | 이 객체의 이 함수를 구독으로 등록. 객체가 사라지면 자동 해제 |
| `.Broadcast(인자)` | 등록된 함수를 전부 호출 |
| `TWeakObjectPtr<T>` | 상대 수명에 영향 안 주는 포인터. 사라지면 `IsValid()`가 `false` |
| `SetTimerForNextTick(람다)` | 다음 프레임에 실행. 초기화 순서 문제를 피할 때 |
| `RestartPlayerAtTransform(컨트롤러, 트랜스폼)` | 지정 위치에 폰 스폰 + 빙의 |
| `FAutoConsoleCommandWithWorld` | 어떤 객체에도 속하지 않는 콘솔 명령 등록 |

### 7-10. 템플릿 원본을 건드린 유일한 곳

`ShooterCharacter.h:178, 185` — `Die()`와 `OnRespawn()`에 `virtual`을 붙였다 (`b2a77e4`).
동작은 바뀌지 않고 자식이 덮어쓸 수 있게 문만 연 변경이다.
대안이었던 "게임모드에서 가로채기"는 죽음에서 세 단계 떨어진 곳에서 결정하게 되어 의도가 코드에 드러나지 않았다.

---

## 8. 상호작용 프레임워크 — E키와 프롬프트

로드맵 2번. 문, 창고 키, 창고, Fixer NPC가 공통으로 쓸 배선이다.
관련 커밋: `c9aafb5`, `4aed58b`, `fb4baaf`, `6b74c27`.

### 8-1. 전체 흐름

```text
[매 틱]
UInteractionComponent::UpdateTarget()
  카메라 시점에서 앞으로 TraceDistance(200cm) 라인 트레이스 (ECC_Visibility)
  맞은 액터가 IInteractable 인가 — Implements<UInteractable>()
  대상이 "바뀌었을 때만" OnTargetChanged 방송
        │
        ▼
UInteractionPromptWidget::HandleTargetChanged(NewTarget)
  대상 있음 → GetInteractionPrompt() → BP_UpdatePrompt() → HitTestInvisible
  대상 없음 → Collapsed

[E 키]
IA_Interact (IMC_Default 에 E)
  → ANeonDistrictCharacter::DoInteract()
  → UInteractionComponent::TryInteract()
       CanInteract(폰) 확인 → Interact(폰)
```

### 8-2. 조각별 역할

| 조각 | 역할 |
|---|---|
| `IInteractable` | 약속 셋 — `GetInteractionPrompt()` `CanInteract()` `Interact()` |
| `UInteractionComponent` | 탐색·방송·E키 처리. 캐릭터에 부착 |
| `UInteractionPromptWidget` | 방송 구독, 문구 전달, 표시·숨김 (`UCLASS(abstract)`). 컨트롤러가 소유하고 빙의 때마다 폰의 컴포넌트에 재바인딩 (8-7) |
| `WBP_InteractionPrompt` | 텍스트 블록 배치, `Update Prompt` 이벤트에서 `SetText` |
| `AInteractableTest` | 배선 검증용 최소 구현체 |

### 8-3. 설계 판단

**인터페이스로 묶는다.** 문·키·창고·NPC는 서로 다른 클래스지만 플레이어에게는 똑같이 "E를 누르는 것"이다.
컴포넌트가 구체 클래스를 하나도 모르게 되고, 템플릿의 `IShooterWeaponHolder`가 픽업·플레이어·NPC를 묶는 방식과 같다.

**캐릭터가 아니라 컴포넌트.** 캐릭터 클래스가 더 커지지 않고, 나중에 다른 폰에도 붙일 수 있다.

**바뀔 때만 방송한다.** 매 틱 방송하면 위젯이 매 프레임 갱신된다. 대상이 같으면 조용히 넘어간다.

**문구는 대상이 제공한다.** 컴포넌트나 위젯이 "문이면 문 열기"를 판단하지 않는다.
각 액터가 자기 문구를 답하므로, 새 상호작용물을 추가해도 위젯 코드는 그대로다.
7절의 "소비자는 미션 내부를 모른다"와 같은 원칙이다.

**C++은 로직, UMG는 겉모습.** `BP_UpdatePrompt`를 `BlueprintImplementableEvent`로 선언만 하고
구현은 블루프린트가 한다. 템플릿 `UShooterUI`의 `BP_UpdateScore`와 같은 형태다.

### 8-4. 걸렸던 것

- **`override`가 오타를 잡아줬다.** `SetupPlayerInputComponents`로 `s`를 하나 더 쓰자 컴파일 에러가 났다.
  `override`가 없었다면 조용히 새 함수가 하나 생기고 부모 함수는 그대로 남아, E키가 안 먹는 원인을 찾기 어려웠을 것이다.
- **`error C4458: 'Instigator'가 클래스 멤버를 숨깁니다.** `AActor`에 이미 `Instigator` 멤버가 있다(`Actor.h:1010`).
  인터페이스의 매개변수 이름을 `InteractingPawn`으로 바꿔 근본에서 해결했다 — 앞으로 만들 문·키·NPC가 전부 `AActor` 자손이라
  그대로 뒀으면 매번 같은 에러를 만났을 것이다.
- **UMG에서 `Set Text` 노드가 안 보인다.** Context Sensitive 검색은 타깃이 정해져야 후보를 보여준다.
  변수를 먼저 그래프에 놓고 그 핀에서 선을 끌어야 나온다.
- **부활 후 프롬프트가 안 뜬다** (9/18, 창고 문 검증 중 발견). 위젯을 캐릭터 `BeginPlay`에서 `IsLocallyControlled()`일 때 만들었는데,
  부활 폰은 빙의 전에 `BeginPlay`가 끝나 조건이 거짓이었다. 8-7에 정리.

### 8-5. 이번에 쓴 API

| | 뜻 |
|---|---|
| `UINTERFACE(MinimalAPI)` + `I` 클래스 | 언리얼 인터페이스는 클래스 두 개. 함수는 `I` 쪽에, 상속도 `I` 쪽 |
| `Implements<UInterface>()` | 인터페이스 구현 여부 검사. `Cast`보다 가벼워 매 틱 검사에 적합 |
| `GetPlayerViewPoint()` | 폰이 아니라 카메라의 위치·방향. 컨트롤러를 거치므로 어떤 폰이든 통한다 |
| `BlueprintImplementableEvent` | C++ 선언, 블루프린트 구현. `.cpp`에 본문을 쓰지 않는다 |
| `ESlateVisibility::HitTestInvisible` | 보이되 입력을 가로채지 않는다. HUD 요소의 기본 선택 |
| `ESlateVisibility::Collapsed` | 공간까지 차지하지 않는다 (`Hidden`은 자리를 남긴다) |
| `NativeDestruct()` | 위젯 정리 시점. 위젯은 파괴되지 않고 내려갔다 올라올 수 있어 여기서 구독을 끊는다 |

### 8-6. 새 상호작용물 추가하는 법

`AInteractableTest`가 최소 템플릿이다. 이것만 하면 프롬프트 표시와 E키 처리는 자동으로 붙는다.

```cpp
class AWarehouseDoor : public AActor, public IInteractable
{
    virtual FText GetInteractionPrompt() const override;                  // "창고 열기" / "잠겨 있다"
    virtual bool CanInteract(APawn* InteractingPawn) const override;      // 키가 없으면 false
    virtual void Interact(APawn* InteractingPawn) override;               // 문 열기
};
```

`CanInteract`는 기본값이 `true`라 조건이 없는 대상은 생략해도 된다.

### 8-7. 부활 후 프롬프트가 안 뜨던 이유 — 폰 `BeginPlay`와 빙의의 순서

처음엔 프롬프트 위젯을 `ANeonDistrictCharacter::BeginPlay`에서 `IsLocallyControlled()`일 때 만들었다.
첫 스폰에서는 되고 부활 후에는 안 됐다. `IsLocallyControlled()`는 폰에 컨트롤러가 붙어 있어야 참인데,
폰의 `BeginPlay`와 컨트롤러가 붙는 `Possess`의 **순서가 첫 스폰과 부활에서 다르다.**

```text
첫 스폰  (월드 시작 전)
  SpawnPlayActor → Login → RestartPlayer
      SpawnDefaultPawn → SpawnActor     ← HasBegunPlay()==false 라 폰 BeginPlay 보류
      Possess(폰)                        ← 컨트롤러 연결
  World->BeginPlay()
      모든 액터 BeginPlay                 ← 이때 폰 BeginPlay. 이미 컨트롤러 있음   ✔

부활  (월드가 도는 중)
  RequestRestart → GameMode->RestartPlayer
      SpawnDefaultPawn → SpawnActor
          PostActorConstruction → BeginPlay 즉시   ← 컨트롤러 없음. IsLocallyControlled()==false   ✘
      Possess(폰)                                  ← 이제야 연결. BeginPlay는 이미 끝났다
```

`SpawnActor`는 월드가 이미 시작됐으면 액터 생성 직후 `BeginPlay`를 바로 부르고, `RestartPlayer`는 폰을 만든 **다음에** `Possess`한다.
첫 스폰에서만 순서가 뒤집혀 있어서 3-A의 부활 검증(체크포인트·무기·미션 단계)은 전부 통과했고, 부활 + 상호작용 조합은 이번이 처음이었다.

같은 구조에 드러나지 않은 문제가 둘 더 있었다: 캐릭터가 위젯을 만들면 죽을 때마다 새 위젯이 생기고 이전 것은 뷰포트에 남아 쌓인다.
`BindToComponent`가 이전 구독을 안 끊어서 재바인딩하면 두 컴포넌트의 방송을 다 받는다.

**고친 것 — 소유자를 컨트롤러로.** 목표 위젯을 컨트롤러에 둔 이유(10절 "컨트롤러는 죽어도 살아남는다")가 그대로 적용된다.
컨트롤러가 위젯을 한 번 만들고, `OnPossess`에서 그 폰의 `UInteractionComponent`에 다시 묶는다. `OnPossess`는 첫 스폰이든 부활이든
빙의할 때마다 불리므로 순서 문제가 없고, 위젯이 하나뿐이라 누적도 없다. `BindToComponent`는 `BoundComponent`가 있으면 `RemoveAll(this)` 후
새로 묶고, `nullptr`이면 Collapsed. 첫 스폰에서는 `OnPossess`가 컨트롤러 `BeginPlay`(위젯 생성)보다 먼저 올 수 있어 — 위 흐름에서
`Possess`가 `World->BeginPlay()` 앞이다 — `BeginPlay`에서 위젯을 만든 직후 `GetPawn()`으로 한 번 더 묶는다. 어느 쪽이 먼저 와도 한 번은 묶인다.

고친 뒤에도 "아예 안 뜬다"였는데, `BP_NeonDistrictPlayerController`에 새 프로퍼티 `InteractionPromptClass`를 지정하지 않아
`if (InteractionPromptClass)`가 조용히 통과한 것이었다. 목표 위젯 때에 이어 두 번째라 `else`에 에러 로그를 두기로 했다.

**교훈**
- 폰의 `BeginPlay`에서 컨트롤러를 전제하지 않는다. 컨트롤러가 필요한 초기화는 `PossessedBy` / `OnPossess` / `PawnClientRestart` 쪽이다.
  3-A의 `SetPlayerDefaults`(무기 지급)가 같은 이유로 그 자리에 있다.
- HUD는 폰이 아니라 컨트롤러가 소유한다. 목표·대화·프롬프트 위젯 셋이 이제 같은 자리에 있다.
- 첫 스폰에서만 통과하는 검증을 조심한다. 부활 후 같은 시나리오를 한 번 더 도는 것을 검증 순서에 넣는다.

관련 커밋: `1215201`.

---

## 9. 코딩 규약 — 헤더 안의 배치 순서

Epic 템플릿(`ShooterCharacter.h`, `ShooterWeapon.h`)이 쓰는 순서를 그대로 따른다.
이 프로젝트 안에서 일관성이 생기고, 다른 언리얼 코드를 읽을 때도 같은 자리에서 같은 것을 찾게 된다.

### 9-1. 원칙 하나

**데이터 → 생성 → 엔진이 부르는 것 → 남이 부르는 것 → 내부.**

읽는 사람은 "이 클래스가 뭘 갖고 있나 → 어떻게 만들어지나 → 언제 동작하나 → 나는 뭘 부를 수 있나" 순서로 궁금해한다. 그 순서대로 놓는다.

### 9-2. 형식

```cpp
#pragma once

#include "CoreMinimal.h"
#include "부모/헤더.h"
#include "인터페이스.h"                    // 구현하는 인터페이스
#include "이파일이름.generated.h"          // 반드시 마지막

// ── 전방 선언 ──
class UBoxComponent;
class AShooterWeapon;

// ── 열거형 / 델리게이트 (이 클래스 전용이면 여기) ──
UENUM()
enum class EMyState : uint8 { ... };

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSomething, AActor*);

/**
 *  클래스가 뭔지 한두 줄
 */
UCLASS()
class CYBERPUNKPROJECT_API AMyActor : public AActor, public IInteractable
{
	GENERATED_BODY()

	// ① 컴포넌트 — private + AllowPrivateAccess. 생성자에서만 만들고 밖에서 교체 안 함
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBoxComponent* Trigger;

protected:

	// ② 설정값 — EditAnywhere / EditDefaultsOnly. 에디터에서 조정하는 것
	UPROPERTY(EditDefaultsOnly, Category="MyActor")
	float Delay = 2.0f;

	// ③ 런타임 상태 — VisibleInstanceOnly 또는 UPROPERTY 없음. 게임 중에 바뀌는 것
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="MyActor")
	EMyState State = EMyState::Idle;

	int32 Count = 0;
	FTimerHandle Timer;
	TWeakObjectPtr<AActor> Target;

public:

	// ④ 생성자
	AMyActor();

	// ⑤ 델리게이트 — 남이 구독하는 것
	FOnSomething OnSomething;

protected:

	// ⑥ 엔진 오버라이드 — BeginPlay, Tick, EndPlay, SetupPlayerInputComponent ...
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:

	// ⑦ 인터페이스 구현
	//~ Begin IInteractable
	virtual FText GetInteractionPrompt() const override;
	virtual void Interact(APawn* InteractingPawn) override;
	//~ End IInteractable

	// ⑧ 공개 API — 남이 부르는 함수. 질의(const) 먼저, 명령 나중
	EMyState GetState() const { return State; }
	bool IsActive() const;

	void Activate();
	void Reset();

protected:

	// ⑨ 자식이 덮어쓰는 것 — 우리가 만든 virtual 확장점
	virtual void OnActivated();

	// ⑩ 내부 구현 — 델리게이트 핸들러, 헬퍼. 밖에서 안 부름
	UFUNCTION()
	void HandleOverlap(...);

	void UpdateSomething();
};
```

### 9-3. 구역별 기준

| 구역 | 접근 | 들어가는 것 | 판단 기준 |
|---|---|---|---|
| ① 컴포넌트 | private | `CreateDefaultSubobject`로 만드는 것 | 생성자 밖에서 바뀌면 안 되는 것 |
| ② 설정값 | protected | `EditAnywhere`, `EditDefaultsOnly` | 에디터에서 손으로 조정하는 것 |
| ③ 런타임 상태 | protected | `VisibleInstanceOnly`, UPROPERTY 없음 | 게임 중에 코드가 바꾸는 것 |
| ④ 생성자 | public | | |
| ⑤ 델리게이트 | public | `FOn...` | 남이 구독해야 하니 public |
| ⑥ 엔진 오버라이드 | protected | `BeginPlay`, `Tick` ... | 엔진만 부르니 protected |
| ⑦ 인터페이스 구현 | public | `//~ Begin` ~ `//~ End` | 인터페이스는 public이 원칙 |
| ⑧ 공개 API | public | 질의 → 명령 순 | 남이 부르는 것 |
| ⑨ 자식 확장점 | protected | 우리가 만든 `virtual` | 자식만 덮어쓰니 protected |
| ⑩ 내부 구현 | protected / private | 핸들러, 헬퍼 | 아무도 안 부름 |

### 9-4. 자주 헷갈리는 것

- **public인가 protected인가** — 딱 하나만 묻는다: *이 클래스 밖에서 부를 일이 있나?* 있으면 public, 없으면 protected, 자식조차 안 부르면 private.
- **`virtual`은 어디에** — 엔진 것(`BeginPlay`)은 ⑥, 인터페이스 것은 ⑦, 우리가 만든 확장점은 ⑧ 또는 ⑨.
  남이 부르기도 하는 `GetObjectiveText`는 ⑧, 자식만 덮어쓰는 `SetupSegment`는 ⑨.
- **`UFUNCTION()` 핸들러** — `AddDynamic`으로 묶는 함수는 `UFUNCTION()`이 필요하고 ⑩에 둔다. 밖에서 부를 일이 없다.
- **접근 지정자가 여러 번 나와도 된다.** `public:` → `protected:` → `public:` 반복이 이상해 보이지만 템플릿도 그렇게 쓴다.
  구역별로 의미가 분명한 것이 지정자를 한 번만 쓰는 것보다 낫다.
- **매개변수 이름은 `AActor` 멤버와 겹치지 않게.** `Instigator`, `Owner`, `Tags` 등은 이미 있다 (8-4 참고).

### 9-5. `.cpp` 순서

헤더 순서를 그대로 따른다. 파일 맨 아래에 `#if !UE_BUILD_SHIPPING`으로 감싼 디버그 콘솔 명령을 둔다.

```cpp
#include "자기 헤더"                 // 반드시 첫 줄
#include "프로젝트 헤더들"
#include "엔진 헤더들"

생성자
엔진 오버라이드
인터페이스 구현
공개 API
확장점
내부 구현

#if !UE_BUILD_SHIPPING
디버그 콘솔 명령
#endif
```

include 경로는 `NeonDistrict/파일.h`처럼 **폴더를 포함한 형태**로 통일한다. `Public/`이 인클루드 경로라 짧게 써도 빌드는 되지만, 어느 모듈 것인지 한눈에 보이게 한다.

---

## 10. 미션 등록부와 목표 HUD — 소비자가 미션을 찾는 통로

로드맵 3-A 5~6단계. HUD·문·NPC가 "지금 미션"을 찾고, 상태가 바뀔 때 목표 문구를 띄운다.
관련 커밋: `7c50bba`, `1576ee1`.

### 10-1. 전체 흐름

```text
ANeonDistrictMission ──Register/Unregister──▶ UMissionRegistry (UWorldSubsystem)
        │                                          │ OnActiveMissionsChanged(미션, 등록여부)
        │ OnStateChanged(미션)                       ▼
        └────────────────────────────────▶ UMissionObjectiveWidget
                                                   │ GetObjectiveText()
                                                   ▼
                                           WBP_MissionObjective (Update Objective → SetText)
                                                   ▲
                                   ANeonDistrictPlayerController::BeginPlay 에서 생성
```

두 겹 구독이다. 등록부에서 "어느 미션을 볼지"를 받고, 그 미션에서 "문구가 바뀌었는지"를 받는다.
게임모드는 여전히 미션을 모르고, 위젯은 미션의 구체 클래스를 모른다 — `GetObjectiveText()`만 부른다.

### 10-2. 조각별 역할

| 조각 | 역할 |
|---|---|
| `EMissionCategory` (`MissionTypes.h`) | `Main` / `Side`. 공용 어휘라 별도 헤더 |
| `UMissionRegistry` | 받은 미션(수락~완료)의 목록. `Register` / `Unregister`, 분류별 질의 `GetActiveMission(Category)` / `GetActiveMissions(Category, Out)`. 상태는 들지 않는다 |
| `ANeonDistrictMission` | 공통 — 상태, 분류, 트리거, 수락/시작/완료, 등록부 등록·해제, `OnStateChanged` |
| `ANeonDistrictMainMission` | 메인만 — `RestartPoint`, 체크포인트, 사망 카운트, 구간 리셋, 랭크. `Category`를 생성자에서 `Main`으로 고정 |
| `UMissionObjectiveWidget` | `TrackedCategory` 하나의 미션을 본다 (`UCLASS(abstract)`) |
| `ANeonDistrictPlayerController` | `MissionObjectiveClass`를 `BeginPlay`에서 생성. 미션도 등록부도 모른다 |
| `WBP_MissionObjective` | 텍스트 블록 배치, `Update Objective` 이벤트에서 `SetText` |
| `BP_NeonDistrictPlayerController` | 템플릿 컨트롤러 복제 → Reparent. IMC·탄약 UI 설정을 이어받는다 |

### 10-3. 위젯의 동작

- `NativeConstruct` — 접힌 채 시작 → `GetWorld()->GetSubsystem<UMissionRegistry>()`로 등록부를 찾아 구독 → 이미 등록된 미션이 있으면 `TrackMission`으로 즉시 반영 (방송은 구독 이전 일을 알려주지 않는다)
- `HandleActiveMissionsChanged(미션, 등록여부)` — 자기 분류가 아니면 무시. 등록이면 `TrackMission(미션)`, 보고 있던 미션의 해제면 `TrackMission(nullptr)`
- `TrackMission` — 이전 미션 구독 해제 → 새 미션 `OnStateChanged` 구독 → `Refresh`
- `Refresh` — `GetObjectiveText()` → `BP_UpdateObjective` → `HitTestInvisible`
- `NativeDestruct` — 등록부·미션 구독 모두 해제

### 10-4. 설계 판단

**주입과 조회를 나누는 기준 — 선택지가 있으면 주입, 유일하면 조회.**
프롬프트 위젯은 `BindToComponent(컴포넌트)`로 받는다. 컴포넌트는 폰마다 하나라 "어느 폰 것이냐"를 만드는 쪽이 정해야 한다.
목표 위젯은 `GetSubsystem`으로 스스로 찾는다. 등록부는 월드에 하나라 고를 게 없고, 주입하면 만드는 쪽에 의존만 하나 는다.
전역으로 하나인 것을 주입하는 건 확장성이 아니라 형식이다.

**볼 대상은 인자가 아니라 속성.** `TrackedCategory`(EditDefaultsOnly)를 WBP 자식이 정한다.
같은 C++ 클래스로 `WBP_MainObjective`(Main)와 `WBP_SideObjectives`(Side)를 만들 수 있고, 만드는 쪽은 둘 다 `CreateWidget`만 한다.

**등록부는 하나, 분류는 속성.** 메인용·사이드용 등록부를 따로 두지 않는다.
메인과 사이드는 같은 종류의 물건이고 다른 건 속성 하나다. 세 번째 분류가 생겨도 enum 값 하나로 끝난다.
"메인은 한 번에 하나"는 등록부가 아니라 미션 쪽 규칙이다 — 등록부에 규칙을 넣기 시작하면 분류마다 예외가 쌓인다.

**메인 미션만의 행동은 클래스로 내린다.** 체크포인트·사망 카운트·구간 리셋은 `ANeonDistrictMainMission`에 있다.
사이드 미션은 `ANeonDistrictMission`을 바로 상속하고, 메인의 체크포인트를 덮어쓸 수 없다는 게 코드로 드러난다.
`Category`는 생성자가 박는다 — enum(등록부 질의용)과 계층(행동 분리용)이 어긋나지 않게.
`Super::StartMission`은 체크포인트·구독·구간 세팅을 마친 뒤 마지막에 부른다. HUD가 방송을 받을 때 미션이 완성 상태여야 한다.

**위젯 소유자는 PlayerController.** 캐릭터는 죽을 때마다 새로 생겨 위젯이 겹친다. 컨트롤러는 살아남으니 한 번만 생성한다.
템플릿이 탄약 카운터를 컨트롤러에서 만드는 것과 같은 자리다. 캐릭터에 남겨뒀던 프롬프트 위젯은 부활 폰의 `BeginPlay`가
빙의보다 먼저 돌아 아예 안 만들어지는 문제가 드러나(8-7) 같은 자리로 옮겼다. 폰에 묶이는 것은 `OnPossess`에서 재바인딩한다.

**등록 시점은 `AcceptMission`.** 처음엔 `StartMission`에 뒀는데, 그러면 Accepted 상태의 "건물에 진입하세요"가 영영 안 보인다.
등록부의 의미를 "진행 중"이 아니라 "받은 미션"으로 잡았다 — 플레이어에게는 수락한 순간부터 목표가 있다.

**미션이 빠져도 마지막 문구를 남긴다.** `CompleteMission`에서 완료 방송 → `Unregister`가 한 호출 안에서 연달아 온다.
빠질 때 접으면 "미션 완료"가 0프레임 만에 사라진다. 구독만 끊고 문구는 다음 미션이 시작할 때까지 유지한다.
같은 이유로 `NotifyStateChanged()`가 먼저, `Unregister`가 나중이다 — 구독이 끊기기 전에 완료 문구를 받아야 한다.

### 10-5. 걸렸던 것

- **`TWeakObjectPtr<ANeonDistrictMission*>`** — `*`가 하나 더 들어갔다. `T`는 클래스 자체이고 포인터는 안에서 만든다.
- **IDE가 끼워 넣은 `#include "ObjectEditorUtils.h"`** — 에디터 전용 헤더라 패키징에서 깨진다. 7절의 `ShaderConductor`와 같은 종류.
- **부모 `.cpp`에서 메인 함수를 옮기다 `NotifyStateChanged()` 정의까지 지웠다** — `LNK2019`. 선언은 남고 정의만 사라지면 컴파일은 통과하고 링크에서 잡힌다.
- **Hot Reload 번호 충돌** — `UnrealEditor-CyberPunkProject maps to -0061 and -0062`. 링크가 중간에 실패하면서 `.pdb`만 남아 UBT가 현재 번호를 못 정했다.
  에디터를 끄고 `Binaries/Win64/`의 번호 붙은 DLL·PDB와 `Intermediate/.../CyberPunkProject/`의 번호 붙은 파일을 지운 뒤 재빌드.
  **새 클래스·새 `UPROPERTY`가 들어가는 빌드는 에디터를 끄고 한다.** 함수 본문만 바뀌면 Live Coding.
- **게임모드 경로가 폴더는 옛것, 이름은 새것으로 섞였다** — `Failed to find object 'Class /Game/Variant_Shooter/Blueprints/BP_ShooterPlayerController.BP_NeonDistrictPlayerController_C'`. 컨트롤러가 안 바뀐 채 플레이됐다.
- **등록 시점을 옮기면서 `Register`가 `Unregister`로 바뀌었다.** 어디서도 등록이 안 되니 위젯은 방송을 못 받고 접힌 채였다.
  로그에 오류가 하나도 없어서, 콘솔 `obj list class=BP_NeonDistrictPlayerController_C` / `obj list class=WBP_MissionObjective_C`로
  컨트롤러와 위젯이 존재하는지부터 갈랐다. 둘 다 2개로 나오는 건 하나가 블루프린트 에디터의 미리보기 객체라 정상이다.

### 10-6. 이번에 쓴 API

| | 뜻 |
|---|---|
| `UWorldSubsystem` | 월드마다 하나 자동 생성, 월드가 끝나면 같이 사라진다. `GetWorld()->GetSubsystem<T>()` |
| `TArray<TWeakObjectPtr<T>>` | 액터 목록을 소유 없이 든다. 원시 포인터로 `Contains` / `Remove` 비교가 된다 |
| `DECLARE_MULTICAST_DELEGATE_TwoParams` | 목록이 되면 "지금 하나가 뭐냐"가 아니라 "무엇이 들어오고 나갔냐"를 보낸다 |
| `CreateWidget(PlayerController, …)` + `AddToPlayerScreen(0)` | 소유자가 컨트롤러. 그 플레이어 화면에 붙고 컨트롤러와 함께 정리된다 |
| `EditDefaultsOnly` | 인스턴스가 아니라 클래스 기본값에서만 편집. WBP 자식이 정하는 설정에 맞다 |
| `obj list class=<클래스>_C` | 콘솔에서 인스턴스 존재 확인. 로그에 오류가 없을 때 첫 번째로 갈라볼 것 |

### 10-7. 검증 순서

플레이 → 아무것도 없음 → `ND.AcceptMission` → "건물에 진입하세요" → 트리거 진입 → "적을 처리하세요"
→ `ND.SetMissionStep 1` → "창고 키를 획득하세요" → `NDKill` → 부활 후 "적을 처리하세요"(구간 리셋). 위젯 하나만 갱신된다.

---

## 11. 구간 리셋 — 데이터 레이어 재활성화

로드맵 3-B. 플레이어가 죽으면 구간 안의 것들(적·키·문)을 처음 상태로 되돌린다.
관련 커밋: `5b2bb21`.

### 11-1. 목표와 접근

되돌릴 대상마다 리셋 코드를 쓰는 대신, 그것들을 런타임 데이터 레이어 `DL_Mission`에 넣고 **레이어를 내렸다 올려서**
디스크에 저장된 상태로 다시 읽게 한다. 되돌릴 대상이 늘어도 누락이 없고, 초기 상태를 `Unloaded`로 두면
"수락 전 적 없음"이 코드 없이 충족된다. 미션 액터·트리거·부활 지점은 이 레이어에 넣지 않는다.

### 11-2. 만든 순서

1. **에디터에서 먼저 검증.** `DL_Mission` 에셋(Runtime) → 레벨 인스턴스, Initial Runtime State = Unloaded →
   물리 켠 테스트 큐브를 레이어에 → 콘솔 `wp.Runtime.SetDataLayerRuntimeState Unloaded / Activated DL_Mission`으로
   큐브가 사라졌다 제자리로 오는 것 확인. 코드가 안 될 때 레이어 문제인지 코드 문제인지 가르기 위해.
2. **`ANeonDistrictMainMission`에 구현.** 창고만의 일이 아니라 메인 미션 공통이라 `AWarehouseMission`이 아닌 부모에.
   `SegmentLayer`(`TObjectPtr<const UDataLayerAsset>`, EditAnywhere)를 레벨의 `WarehouseMission`에서 지정.
3. **대기 조건을 세 번 틀리고 네 번째에 맞춤** (11-3).

### 11-3. 진짜 원인 — "월드에서 빠짐"과 "메모리에서 사라짐"은 다르다

| 시도 | 대기 조건 | 결과 |
|---|---|---|
| `IsAllStreamingCompleted()` | 대기 중인 스트리밍 없음 | 0.1초 만에 통과, 큐브 밀린 자리 그대로 |
| 액터 순회 `ContainsDataLayer` | 레이어 액터가 월드에 없음 | 동일 |
| `GetLoadedLevel()` | 셀 스트리밍 레벨이 레벨을 안 듦 | 동일 |
| **패키지가 메모리에 없음** + GC 요청 | 셀 레벨이 실제로 지워짐 | 0.2초, 제자리 복귀 |

세 시도 모두 "월드에서 빠졌는가"까지만 봤다. 엔진 소스에서 확인한 실제 동작:

- `SetDataLayerRuntimeState(Unloaded)`는 셀 스트리밍 레벨에 플래그만 켜고, 제거는 다음 스트리밍 업데이트가 한다.
  같은 프레임에 `Activated`가 오면 플래그만 도로 뒤집힌다.
- 내려간 레벨이 GC 전이면 엔진이 **그 레벨을 그대로 재사용**한다 (`WorldPartitionLevelStreamingDynamic.cpp`의
  "Reuse existing Level"). 셀 경계를 왔다갔다할 때 디스크를 다시 읽지 않으려는 최적화.
- 일반 레벨 스트리밍은 내려간 뒤 GC를 강제하지만(`GLevelStreamingForceGCAfterLevelStreamedOut = 1`),
  **World Partition은 이걸 끈다** (`WorldPartitionSubsystem.cpp`). 셀이 많아 매번 GC 히치를 낼 수 없어서.

그래서 GC 주기(약 60초) 안에 올리면 항상 밀린 큐브를 포함한 옛 레벨이 돌아왔다. 콘솔로 됐던 건 두 명령 사이에
우연히 GC가 돌았던 것이다.

### 11-4. 최종 동작

```text
SetupSegment()
  ① 레이어 셀의 레벨 패키지 이름을 기억   — 내려간 스트리밍 레벨은 월드 목록에서 빠져 나중엔 못 찾는다
  ② Unloaded, 0.1초 타이머 시작

ActiveSegmentWhenUnloaded()   0.1초마다
  ③ 셀이 아직 월드에 붙어 있으면 대기
  ④ 패키지가 아직 메모리에 있으면 GEngine->ForceGarbageCollection(true) 요청 후 대기   — 다음 틱에 GC
  ⑤ 둘 다 통과 → 타이머 중지, Activated
```

World Partition이 끈 GC를 이 레이어에 한해 우리가 대신 거는 셈이다. 첫 시작은 레이어가 원래 `Unloaded`라 ③④가
즉시 통과해 분기 없이 같은 코드로 돈다. 빠르게 두 번 죽어도 같은 타이머 핸들이 교체되고 `Unloaded`는 멱등이라 안전하다.
`ForceGarbageCollection`은 요청만 하고 실제 GC는 다음 틱이라, 요청하고 `return` → 다음 틱에 다시 확인하는 구조다.

**택하지 않은 대안** — 콘솔 변수 `LevelStreaming.ShouldReuseUnloadedButStillAroundLevels 0`으로 재사용을 끄는 것.
GC 전까지 셀이 안 올라와 최대 60초 빈 구간이 생기고, 모든 셀의 스트리밍 성능에 영향을 준다. 미션 하나 때문에 전역을 바꿀 일이 아니다.

### 11-5. 어떻게 찾았나 — 혼자 도는 테스트

PIE를 직접 못 돌려서 `-NDSegmentTest` 인자로 실행하면 `BeginPlay`에서 타이머로 도는 임시 테스트를 넣고
`-game -unattended`로 돌려 로그만 읽었다: 2초 레이어 올리기 → 5초 큐브 500cm 밀기 → 6초 `SetupSegment()` →
0.05초마다 스트리밍 레벨 상태·큐브 이름·위치 덤프 → 12초 종료.

```text
t=6.11  cellLevels=0 cube=none                                   ← 내려감
t=6.15  cube=StaticMeshActor_..._1920928583  loc=X=-5471          ← 같은 객체, 밀린 위치
```

**재활성화 전후 액터 이름이 같다** = 새로 만든 게 아니라 같은 객체. 여기서 재사용 코드를 찾아 들어갔다.
고친 뒤 위치가 저장값(X=-5970)으로 돌아온 것을 확인하고 테스트 코드는 통째로 제거했다.

### 11-6. 걸렸던 것

- **에디터를 켜둔 채 외부에서 세 번 빌드**해서 번호 DLL이 다시 쌓였고, 에디터가 `-0024`를 물고 있어
  `Unable to delete hot-reload file`이 났다. 에디터 끄고 번호 DLL 정리 후 재빌드. 외부 빌드는 에디터를 먼저 끈다.
- 헤더에서 `ClearTimer` 줄이 빠진 채 빌드된 적이 있다. 타이머가 `Activated` 이후에도 0.1초마다 돌았지만
  ③에서 `return`해 조용히 계속 돌기만 해서 로그로는 안 드러났다.

### 11-7. 이번에 쓴 API

| | 뜻 |
|---|---|
| `UDataLayerManager::GetDataLayerManager(this)` / `SetDataLayerRuntimeState(Asset, State)` | 런타임 데이터 레이어 상태 전환. `Unloaded / Loaded / Activated` |
| `UWorld::GetStreamingLevels()` + `UWorldPartitionLevelStreamingDynamic` | 셀 하나가 스트리밍 레벨 하나. `GetWorldPartitionRuntimeCell()->ContainsDataLayer(Asset)`로 레이어 소속 확인 |
| `ULevelStreaming::GetWorldAssetPackageFName()` | 셀 레벨의 패키지 이름. 내려간 뒤에도 이 이름으로 메모리 존재를 물을 수 있다 |
| `StaticFindObjectFast(UPackage::StaticClass(), nullptr, Name, EFindObjectFlags::None, RF_NoFlags, EInternalObjectFlags::Garbage)` | 마지막 인자가 "Garbage 표시된 건 제외". 엔진의 재사용 코드가 같은 호출로 판단하니 같은 기준으로 본다 |
| `GEngine->ForceGarbageCollection(true)` | 다음 틱에 전체 GC. 평소 주기(약 60초)를 한 번 앞당긴다 |
| `wp.Runtime.SetDataLayerRuntimeState <State> <Layer>` | 콘솔에서 레이어 상태 전환. 코드 전에 레이어 자체를 검증할 때 |

---

## 12. Fixer NPC와 대화 — 수락, 분기, 완료

로드맵 4번 ①~④. 대로의 Fixer에게 말을 걸어 미션을 받고, 아이템을 들고 돌아오면 완료·랭크.
`ND.AcceptMission` 콘솔 명령을 실제 플레이로 대체한다. 관련 커밋: `979498e`, `a6d19ca`, `b7df929`, `6ca09ad`.

### 12-1. 전체 흐름

```text
플레이어 E
  → UInteractionComponent::TryInteract  →  AFixerNPC::Interact
       PickStartRow()                      미션 상태로 시작 행 선택
       PC->StartDialogue(테이블, 행)        위젯 열고 이동·시점 잠금
       위젯->OnEffect ← ApplyEffect        줄의 Effect를 미션에 적용할 준비
플레이어 E (대화 중)
  → ANeonDistrictCharacter::DoInteract   →  PC->AdvanceDialogue
       UDialogueWidget::Advance            현재 줄 Effect 방송 → 다음 행 → 없으면 Finish
       OnFinished → 컨트롤러가 입력 복구
```

### 12-2. 조각별 역할

| 조각 | 역할 |
|---|---|
| `AFixerNPC` | `IInteractable`. 캡슐(Visibility 차단) + 스켈레탈 메시(NoCollision). `Mission` 참조(EditInstanceOnly), 대사 테이블, 상태별 시작 행 4개. 시작 행을 고르고 Effect를 미션에 적용 |
| `FDialogueLine` (`MissionDialogueTypes.h`) | 데이터 테이블 행: Speaker, Text, NextRow, Effect(None / AcceptMission / CompleteMission) |
| `DT_FixerDialogue` | `Intro_1~3` → 수락, `InProgress_1`, `Return_1` → 완료, `Completed_1` |
| `UDialogueWidget` (abstract) | 현재 행을 들고 `Advance()`. 미션을 모르고 `OnEffect` / `OnFinished` 델리게이트만 |
| `WBP_Dialogue` | `SpeakerText` · `LineText`, `Update Line` 이벤트 |
| `ANeonDistrictPlayerController` | 위젯 소유(한 번 만들어 재사용), `StartDialogue` / `AdvanceDialogue` / `IsInDialogue`, `SetIgnoreMoveInput` / `SetIgnoreLookInput` 잠금·복구 |
| `ANeonDistrictCharacter::DoInteract` | 대화 중이면 E = 다음 줄, 아니면 E = 상호작용 |
| `ANeonDistrictMission::CanComplete()` | virtual, 기본 false. 창고는 `InProgress && Step == ItemAcquired`(처음엔 `Returning`, `7531d99`에서 단계 재편). 대사 선택과 `CompleteMission()` 가드 양쪽에서 쓴다 |

### 12-3. 수락 — E 한 번이 HUD까지 닿는 경로

```text
E (Intro_3 "알았어"를 넘김)
  DoInteract → PC->AdvanceDialogue → UDialogueWidget::Advance
    CurrentEffect == AcceptMission → OnEffect.Execute → AFixerNPC::ApplyEffect
      → Mission->AcceptMission()
           State: NotAccepted → Accepted
           Registry->Register(this) → OnActiveMissionsChanged(미션, true)
             → UMissionObjectiveWidget: 분류 일치 → TrackMission → Refresh → "건물에 진입하세요"
           NotifyStateChanged()
    ShowRow(NextRow = None) → Finish → Collapsed → OnFinished → 입력 복구
```

| 시점 | `State` | 등록부 | HUD |
|---|---|---|---|
| 대화 중 (Intro_1~3 표시) | `NotAccepted` | 없음 | 없음 |
| Intro_3을 **넘기는 순간** | `Accepted` | 등록 | "건물에 진입하세요" |
| 트리거 진입 | `InProgress` | 그대로 | "적을 처리하세요" |

**Effect는 줄을 넘길 때 적용된다.** "알았어"가 떠 있는 동안은 아직 미수락. 위젯이 닫히는 것과 HUD가 뜨는 것이 같은 E 한 번에 일어난다.
`Register`가 `StartMission`이 아닌 `AcceptMission`에 있는 이유가 여기 있다 (10절) — 등록부는 "진행 중"이 아니라 "받은 미션".

안전장치: `AcceptMission()`은 `NotAccepted`일 때만 동작. `StartDialogue`는 대화 중이면 `nullptr`. `CanInteract`는 `Mission == nullptr`면 false —
다만 로그 없이 조용히 무시라 배치 실수를 잡기 어렵다 (12-6).

### 12-4. 상태에 따른 대화 분기

NPC는 미션에게 두 가지만 묻는다 — `GetState()`와 `CanComplete()`. 그걸로 시작 행 하나를 고르고, 그 뒤는 테이블의 `NextRow`가 잇는다.

| 미션 상태 | `CanComplete()` | 시작 행 | 대사 | 줄 끝 Effect |
|---|---|---|---|---|
| `NotAccepted` | — | `Intro_1` → `Intro_2` → `Intro_3` | 의뢰 → "알았어" | `AcceptMission` |
| `Accepted` / `InProgress` | false | `InProgress_1` | "아직이야? 서둘러" | — |
| `InProgress` | **true** (`ItemAcquired`) | `Return_1` | "가져왔군. 값은 약속대로" | `CompleteMission` |
| `Completed` | — | `Completed_1` | "수고했어" | — |

```cpp
FName AFixerNPC::PickStartRow() const
{
	switch (Mission->GetState())
	{
	case EMissionState::NotAccepted: return IntroRow;
	case EMissionState::Completed:   return CompletedRow;
	default:                         return Mission->CanComplete() ? ReturnRow : InProgressRow;
	}
}
```

**분기의 두 층.** 1층은 미션 공통 상태(`EMissionState`)라 `switch`로 직접 본다. 2층 "아이템을 들고 왔나"는 창고 미션의
`Step == ItemAcquired`이지만 NPC는 그걸 모른다 — 미션에 virtual 질의 `CanComplete()`를 두고 자식이 답한다.
다른 미션이 오면 그 미션이 자기 조건으로 답하고 NPC 코드는 그대로다. "소비자는 미션 내부를 모른다"(7절)의 연장.

**`CanComplete()`가 두 곳에서 쓰이는 이유.** `PickStartRow`(Return_1을 고를지)와 `CompleteMission()`(실행할지, false면 return)이
같은 함수를 보니 **둘이 어긋날 수 없다.** 대사가 "가져왔군"으로 갈라졌다면 완료도 반드시 되고, 테이블에서 `InProgress_1`에
실수로 `CompleteMission`을 넣어도 가드가 막는다 — 조기 완료는 데이터로 일으킬 수 없다.

**왜 시작 행만 코드가 고르나.** 대안은 행마다 "어느 상태에서 보이나" 조건 필드를 두고 위젯이 걸러 가는 방식(로드맵 원안)이었다.
그러면 위젯이 미션 상태를 알아야 해서 "위젯은 미션을 모른다"가 깨지고, 조건이 행마다 흩어져 "지금 상태에서 무슨 대사가 나오지"를
한눈에 못 본다. 시작 행 4개를 NPC 프로퍼티로 두면 분기가 함수 하나에 모이고 나머지는 단순 연결 리스트다.
상태가 늘면 프로퍼티와 `case` 하나씩 — 사이드 미션이 들어와도 같은 NPC 클래스로 된다.

### 12-5. 설계 판단

**NPC가 주는 미션은 레벨에서 지정한다.** 등록부는 "받은 미션"만 들어 수락 전 미션은 못 찾는다.
`EditInstanceOnly TObjectPtr<ANeonDistrictMission>`로 배치된 NPC에 `WarehouseMission`을 꽂는다. 추상 부모 타입이라 어떤 미션이든 꽂힌다.

**미션 상태 변화는 데이터에 있다.** 어느 줄에서 수락·완료되는지는 행의 `Effect`. 기획이 대사 구조를 바꿔도 코드를 안 건드린다.

**위젯은 미션을 모른다.** Effect를 델리게이트로 던지고 적용은 NPC가 한다. `OnEffect`는 단일 델리게이트(`DECLARE_DELEGATE`)라
NPC가 `BindUObject`하면 이전 것이 교체돼, 위젯을 재사용해도 두 번째 NPC의 Effect가 첫 NPC로 가지 않는다.

**입력 잠금은 컨트롤러의 `SetIgnoreMoveInput` / `SetIgnoreLookInput`.** 엔진 내장, 카운트 방식이라 `true`/`false` 짝만 맞추면 된다.
E는 계속 들어오니 캐릭터가 `IsInDialogue()`로 분기 — `IA_Interact` 매핑을 그대로 쓰고 키를 하드코딩하지 않는다. 사격은 매핑 컨텍스트를 빼서 막는다 (12-10).

**시작 행 이름이 프로퍼티인 이유.** 클래스에 박으면 두 번째 NPC에서 코드를 고쳐야 한다. 인스턴스가 정하면 같은 클래스로 다른 대사.

**완료 문구에 랭크.** `ANeonDistrictMainMission::GetObjectiveText` 오버라이드, `FText::Format` + `NSLOCTEXT`.
값이 들어가는 첫 문구라 현지화 가능한 형태로. 완료 후 등록 해제되므로 HUD는 "미션 완료 — 랭크 S"를 마지막 문구로 남긴다.

### 12-6. 걸렸던 것

- **`DialogueTypes.h` 이름 충돌** — 엔진에 `Sound/DialogueTypes.h`가 있어 UHT가 `.generated.h` 중복으로 거부. `MissionDialogueTypes.h`로 변경.
- **새 클래스를 Live Coding으로 추가** — `FixerNPC`의 인터페이스 목록이 등록되지 않아 `Implements<UInteractable>()`이 false.
  로그 `Could not find existing class FixerNPC ... assuming new`, `Can't find class descriptor`. 에디터 재시작으로 해결.
- **배치한 NPC에 Mission·메시 미지정, Z=0** — `CanInteract` false로 E 무시(로그 없음), 메시 없어 투명, 캡슐 절반이 바닥 아래라
  카메라 높이 트레이스가 위를 지나감. 액터 파일을 읽어 참조가 없는 것을 확인.
- **레벨 미저장** — PIE는 메모리 값으로 돌아서 되는데 액터 파일엔 Mission / DialogueTable 참조가 없었다.
  두 번 다 커밋 전에 파일 mtime과 내용으로 잡음. 에디터에서 설정만 바꾸고 저장을 안 하면 PIE는 되고 커밋은 빈 채가 된다.
- **`PickStartRow`의 `Accepted → CompletedRow`** — 수락 직후 "수고했어". `Completed` 케이스로 수정, `Accepted`는 `default`.

### 12-7. 이번에 쓴 API

| | 뜻 |
|---|---|
| `FTableRowBase` + `USTRUCT(BlueprintType)` | 데이터 테이블 행 구조의 조건. 에디터의 행 구조 목록에 나온다 |
| `UDataTable::FindRow<T>(Name, Context)` | 행 조회. 두 번째 인자는 못 찾았을 때 경고 로그의 문맥 문자열 |
| `meta = (RequiredAssetDataTags = "RowStructure=/Script/CyberPunkProject.DialogueLine")` | 디테일 드롭다운에 그 행 구조의 테이블만 보이게 |
| `EditInstanceOnly` | 클래스 기본값이 아니라 배치된 인스턴스에서만 편집. 레벨 액터 참조에 맞다 |
| `DECLARE_DELEGATE` / `BindUObject` / `ExecuteIfBound` | 단일 델리게이트. 묶으면 교체, 안 묶였으면 조용히 통과 |
| `APlayerController::SetIgnoreMoveInput / SetIgnoreLookInput` | 이동·시점 입력 잠금. 카운트 방식 |
| `APawn::GetController<T>()` | 폰에서 컨트롤러를 타입으로 |
| `FText::Format` + `NSLOCTEXT` | 값이 들어가는 문구를 현지화 가능한 형태로 |

### 12-8. 검증 순서

플레이 → NPC E → 세 줄 → 수락, HUD "건물에 진입하세요" → 다시 E → "아직이야?" → 트리거 진입 → `ND.SetMissionStep 3` →
NPC E → "가져왔군" → HUD "미션 완료 — 랭크 S". 중간에 `NDKill` 넣으면 A. `ND.SetMissionStep 1`에서 말 걸면 "아직이야?"여야 한다.

남은 것 (4번): 완료 후 NPC 이동. 통화 연출의 카메라는 12-11에, 통화 UI 겉모습은 7번 HUD에.

### 12-9. Fixer 복귀와 완료 처리

```text
데이터칩 획득  →  AdvanceTo(ItemAcquired)  →  OnStateChanged
                                               ├ HUD     "Fixer에게 돌아가세요"
                                               └ 셔터    개방 (복귀 지름길)
셔터 통과 → 대로 → Fixer
E  →  AFixerNPC::PickStartRow
        State == InProgress, CanComplete() == true   →  "Return_1"  ("가져왔군. 값은 약속대로")
E  →  Advance → Effect = CompleteMission → AFixerNPC::ApplyEffect
        → Mission->CompleteMission()
             CanComplete() 재확인 (가드)
             State = Completed
             NotifyStateChanged()   → HUD "미션 완료 — 랭크 S"       ← 먼저
             Registry->Unregister   → HUD 구독 해제, 마지막 문구 유지  ← 나중
이후 E  →  State == Completed → "Completed_1" ("수고했어")
```

**완료 시점에 일어나는 일**

| 순서 | 코드 | 결과 |
|---|---|---|
| 1 | `State = Completed` | `IsInProgress()` false → 픽업·문·적 카운트 전부 비활성 |
| 2 | `NotifyStateChanged()` | HUD `Refresh` → `ANeonDistrictMainMission::GetObjectiveText` → `"미션 완료 — 랭크 {GetRank()}"` |
| 3 | `Registry->Unregister(this)` | `OnActiveMissionsChanged(미션, false)` → HUD가 구독만 끊고 문구는 남김 |

2와 3의 순서가 핵심이다. 반대면 HUD가 구독을 끊은 뒤 완료 방송이 와서 랭크를 못 본다. HUD가 해제 시 화면을 접지 않는 이유도 여기 —
2·3이 한 호출 안에서 연달아 오니 접으면 "미션 완료"가 0프레임 만에 사라진다 (10-4).

**랭크** — `ANeonDistrictMainMission::GetRank()`, 사망 횟수 `0 → S, 1 → A, 2 → B, 3+ → C`. `DeathCount`는 `StartMission`에서 0,
`HandlePlayerDied`에서 `++`.

**부활과 완료가 섞이지 않는 이유** — 죽으면 `HandlePlayerDied → SetupSegment → Step = Fighting`이라 `CanComplete()`가 false로 돌아간다.
칩을 들고 죽으면 칩·키·셔터가 레이어 재활성화로 원상복구되고 다시 싸워야 한다. "죽으면 구간 처음부터"가 완료 조건에도 그대로 적용된다.

**아직 없는 것** — 완료 후 NPC 이동(기획서 "npc 위치 옮기기", `Completed` 방송을 NPC가 구독), 종료 컷씬(8번, 같은 방송에 시퀀스 재생),
미션 완료 창(7번, 지금은 목표 문구 한 줄이 랭크를 겸함).

**검증 (9/22, 콘솔 없음)** — Fixer E → 수락 → 적 2명 처치 → 키 드롭·획득 → 문 → 칩(셔터 개방) → 셔터 통과 → Fixer E → "가져왔군" → E →
HUD "미션 완료 — 랭크 S" + 로그 `[Mission] 완료` → 다시 E → "수고했어". 중간에 한 번 죽으면 A. 관련 커밋: `6ca09ad`, `d0ce476`, `b47be2d`.

### 12-10. 대화 중 입력 잠금 — 사격 차단

통화 중에도 좌클릭으로 총이 나갔다. `SetIgnoreMoveInput` / `SetIgnoreLookInput`은 이동·시점만 막고 무기 입력은 건드리지 않는다.
관련 커밋: `74eb05c`.

| 방법 | 평가 |
|---|---|
| `DoStartFiring`을 오버라이드해서 `IsInDialogue()`면 무시 | 템플릿에서 `virtual`이 아님. `virtual` 붙이면 템플릿 수정 (지금까지 `Die` / `OnRespawn` 둘뿐) |
| 무기 쪽에서 컨트롤러에 물어보기 | 무기가 대화를 알게 됨. 방향이 거꾸로 |
| **대화 중 `IMC_Weapons` 매핑 컨텍스트를 뺀다** | Enhanced Input이 원래 이렇게 쓰라고 만든 것. 사격·무기 전환이 한꺼번에 막히고 템플릿 무수정 |

```text
StartDialogue
  SetIgnoreMoveInput(true) / SetIgnoreLookInput(true)
  DialogueBlockedContexts 마다 Subsystem->RemoveMappingContext      ← IMC_Weapons (IA_Shoot, IA_SwapWeapon)
  Pawn->DoStopFiring()                                                ← 이미 누르고 있던 사격 끊기
HandleDialogueFinished
  SetIgnoreMoveInput(false) / SetIgnoreLookInput(false)
  DialogueBlockedContexts 마다 Subsystem->AddMappingContext(Ctx, 0)  ← 템플릿이 넣을 때와 같은 우선순위
```

`DialogueBlockedContexts`는 컨트롤러의 `EditDefaultsOnly TArray<TObjectPtr<UInputMappingContext>>` — BP에서 `IMC_Weapons`를 꽂는다.
`IMC_Default`는 안 뺀다: E(다음 줄)가 거기 있고, 이동은 이미 막혀 있다.

**왜 `DoStopFiring`이 따로 필요한가.** 매핑 컨텍스트를 빼면 그 액션의 입력이 더 이상 안 들어오는 것이지, 이미 눌린 채 진행 중인 액션에
`Completed`가 오지는 않는다. 좌클릭을 누른 채 E로 대화를 시작하면 `bIsFiring = true`인 채로 남아 자동사격이 계속된다.
그래서 대화 시작 순간 `DoStopFiring()`을 직접 부른다 — 템플릿이 `BlueprintCallable public`으로 열어둔 함수라 그대로 쓴다.

**걸렸던 것**

| 문제 | 원인 | 해결 |
|---|---|---|
| `error C4458: 'Character'가 클래스 멤버를 숨깁니다` | `APlayerController`에 `Character` 멤버가 이미 있음. 8절 `Instigator`와 같은 종류 | 지역 변수 `ShooterPawn` |
| 대화 시작하면 총알이 **계속** 나감 | `DoStopFiring` 자리에 `DoStartFiring`을 씀. 시작 순간 사격을 켜고, 컨텍스트가 빠져 놓아도 `Completed`가 안 와 멈출 길이 없음 | 함수 이름 수정 |

두 번째 건은 증상이 원인을 그대로 말해줬다 — "차단했는데 멈추는 게 아니라 계속 나간다"는 차단이 안 된 게 아니라 **켜진** 것.

| | 뜻 |
|---|---|
| `ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())` | 로컬 플레이어의 입력 서브시스템 |
| `RemoveMappingContext` / `AddMappingContext(Ctx, Priority)` | 컨텍스트 단위로 입력 켜고 끄기 |
| `GetPawn<T>()` | 컨트롤러에서 폰을 타입으로 |

### 12-11. 대화 카메라 — 누가 각도를 드는가

`StartDialogue`에 남겨둔 메모("카메라 무빙 추가 — 미션마다 카메라 위치 다르게 설정할 수 있는지? 캐릭터에 붙여야 하는지?")에 대한 답이다.
관련 커밋: `16e3e08`.

**결론 — 미션도 캐릭터도 아니고 NPC.** "어떤 각도로 이 사람과 대화하나"는 대화 상대마다 다른 것이다.

| 후보 | 왜 아닌가 |
|---|---|
| 미션이 든다 | 한 미션에 NPC가 둘이면 각도가 하나뿐. 반대로 한 NPC가 여러 미션을 주면 미션마다 같은 각도를 중복 지정 |
| 캐릭터(플레이어)가 든다 | 플레이어는 어디서 말을 걸지 모른다. 부스를 비추는 각도는 부스가 안다 |
| **NPC가 든다** | 각도가 그 NPC의 배치와 함께 레벨에 남는다. NPC가 늘어도 코드 불변 |

```text
AFixerNPC
  └ UCameraComponent DialogueCamera      생성자에서 만들고, 뷰포트에서 각도 조정

AFixerNPC::Interact
  PC->StartDialogue(테이블, 시작행, this)          ← 자기 자신을 view target으로
ANeonDistrictPlayerController::StartDialogue
  위젯 열기 + 이동·시점·무기 입력 잠금
  ViewTarget 있으면 SetViewTargetWithBlend(ViewTarget, BlendTime, EaseInOut, 2)
HandleDialogueFinished
  입력 복구 + SetViewTargetWithBlend(GetPawn(), ...)
```

**전환 대상은 컴포넌트가 아니라 액터다.** `SetViewTargetWithBlend`는 `AActor*`를 받고, 그 액터에 `UCameraComponent`가 있으면
엔진이 자동으로 그것을 쓴다. 그래서 카메라를 따로 꺼내지 않고 NPC 액터 자체를 넘긴다.

**`ViewTarget`이 `nullptr`이면 화면을 안 바꾼다.** 기본값이 `nullptr`이라 카메라 연출이 없는 NPC도 그대로 동작하고 기존 호출부도 안 깨진다.

**시점 입력 잠금이 여기서 의미가 커진다.** 안 잠그면 카메라가 NPC를 비추는 동안 마우스가 폰을 계속 돌려서, 복귀했을 때 엉뚱한 방향을 본다.
대화 시스템을 만들 때 넣어둔 `SetIgnoreLookInput`이 카메라 전환의 전제가 됐다.

**복귀 대상은 `GetPawn()`.** 대화 중엔 이동·사격이 잠겨 폰이 바뀔 일이 없지만, `nullptr`이면 전환을 건너뛴다 — 부활이 알아서 새 폰을 view target으로 잡는다.

**블렌드 값은 컨트롤러 프로퍼티** (`DialogueCameraBlendTime`, 기본 0.6초 `VTBlend_EaseInOut` Exp 2).
통화 느낌으로 각지게 하려면 `VTBlend_Linear` 0.2초 쪽이 나을 수 있고, 값만 바꾸면 된다.

**아직 없는 것.** 뷰포트에서 맞춘 각도의 레벨 저장(지금 저장소에는 생성자 기본값만), 통화 UI 연출(카메라가 아니라 `WBP_Dialogue`의 겉모습 — 7번 HUD),
NPC 애니메이션.

---

## 13. 적 — NavMesh 확인, 생성, 순찰

로드맵 5번. 순서대로 World Partition 환경에서 NavMesh가 되는지 확인하고(9/21), 적을 레이어에 배치해 마지막 적이 키를 떨어뜨리게 하고(9/22),
로밍을 지정 경로 순찰로 바꿨다(9/22). 관련 커밋: `15891f5`(NavMesh), `b47be2d`(적 배치·키 드롭), `5f19453`(순찰).
배치 절차는 `docs/enemy_placement.md`.

### 13-0. World Partition 환경 NavMesh 확인

**왜 먼저 했나.** 로드맵 3주차 최대 위험 — "WP 환경에서 블록아웃 전역에 NavMesh가 안 깔리면 5번 전체가 멈춘다".
적을 붙이기 전에 30분짜리 확인으로 위험을 미리 없앴다. 결과는 **통과**: 템플릿 `BP_ShooterNPC`를 플레이어와 다른 셀에 놓고
플레이하니 걸어와서 교전했다.

| 단계 | 한 것 | 결과 |
|---|---|---|
| 1 | `NavMeshBoundsVolume`을 플레이 구역(X −7500~+3500, Y −1500~+1500, 높이 ~1000) 전체로 | `RecastNavMesh-Default` 자동 생성 |
| 2 | 뷰포트 `P` | 아무것도 안 뜸 |
| 3 | Build → Build Paths | 로그 `build time: 0.00s` — 아무것도 안 구움 |
| 4 | PIE + 적 | `Unable to find RecastNavMesh instance`, 적이 안 걸어옴 |
| 5 | World Partition 창에서 플레이 구역 **Load Region** → Build Paths | `build time: 0.05s`, `P`에 초록, 적이 걸어옴 |

**원인 — WP 에디터의 Build Paths는 "로드된 리전"만 굽는다.** `NavigationSystem.cpp`의 `CheckToLimitNavigationBoundsToLoadedRegions`:

```cpp
const TArray<FBox> LoadedWorldPartitionRegions = WorldPartition->GetUserLoadedEditorRegions();
```

WP 레벨에서 NavMesh가 `bIsWorldPartitioned`이면 에디터 빌드는 볼륨 전체가 아니라 **볼륨 ∩ 사용자가 로드한 리전**만 굽는다.
WP 에디터에서 액터는 로드한 리전에만 존재하므로 안 로드된 셀의 바닥은 구울 수 없다. 리전을 하나도 안 로드한 채 눌러서 `0.00s`였다.

**`NavDataChunkActor`가 안 생긴 이유.** 처음엔 WP 방식이면 NavMesh가 셀 단위 청크 액터로 쪼개진다고 예상했는데 안 생겼다.
5.8에서는 WP 레벨이라도 기본은 **항상 로드되는 `RecastNavMesh` 액터 하나에 타일 전부 저장**이고, 청크 분할은 옵션
(`bAllowWorldPartitionedNavMesh`)이다. 저장된 액터 파일이 13KB로 커진 것이 그 증거.

**청크 스트리밍을 택하지 않은 이유.** 청크 스트리밍은 NavMesh를 셀마다 쪼개 셀이 로드될 때 그 조각만 올리는 방식이다.
수 km 오픈월드용이고, 단점은 경로가 셀 경계를 넘을 때 목적지 셀이 아직 안 올라와 있으면 "길 없음"이 되는 것 — 로드맵이 걱정한 위험이
정확히 이것이다. 150×150m에 13KB면 쪼갤 이유가 없고 쪼개면 그 위험만 얻는다. 택하지 않은 것도 판단이다.

**정석 — 커맨드릿.** 블록아웃이 바뀔 때마다 "리전을 전부 로드했는지"를 사람이 기억하는 건 재현성이 없다.

```text
UnrealEditor-Cmd.exe <프로젝트> <맵> -run=WorldPartitionBuilderCommandlet -Builder=WorldPartitionNavigationDataBuilder -AllowCommandletRendering
```

창 없는 에디터가 맵의 셀을 순서대로 전부 로드해 가며 굽고 저장한 뒤 꺼진다. HLOD도 같은 빌더 체계(`-Builder=WorldPartitionHLODsBuilder`)라
5주차에 다시 만난다. 지금은 확인이 목적이라 Build Paths로 끝냈고, 블록아웃이 굳으면 한 번 돌려 기록으로 남긴다.

**남은 메모.** `Unable to find RecastNavMesh instance while trying to create UCrowdManager instance`는 월드 정리 시점에 찍히고
동작엔 영향 없다. 문·셔터가 열릴 때 그 자리에 길이 뚫려야 하면 `RecastNavMesh`의 Runtime Generation을 `Dynamic Modifiers Only`로 —
아직 안 바꿈, 지금은 문 뒤가 막다른 창고라 적이 들어갈 일이 없다. 블록아웃을 고친 뒤엔 **플레이 구역 전체 Load Region → Build Paths → 저장**.

### 13-1. 적 생성 — 스포너 없이 배치 + 레이어

**한 줄.** 적은 레벨에 직접 배치되고, `DL_Mission` 레이어가 올라올 때 "생성"되며, 내려갈 때 사라진다. 생성·리셋 코드가 따로 없다.

**왜 스포너를 쓰지 않나.** 템플릿 `BP_ShooterNPCSpawner`는 주기적으로 적을 만들고 죽으면 다시 만든다 — 데스매치용 무한 리스폰이다.
우리 미션은 "이 구간의 적 N명을 다 잡으면 다음"이라 개수가 고정이고 죽으면 처음 상태로 돌아가야 한다.

| | 스포너 | 배치 + 레이어 |
|---|---|---|
| 개수 | 코드/타이머가 결정 | 레벨에 놓인 만큼 |
| 위치·경로 | 스폰 지점 하나 | 액터마다 에디터에서 지정 |
| 죽었을 때 리셋 | 살아 있는 적 정리 + 재스폰 코드 | 레이어 내렸다 올리기 (11절) |
| 수락 전 | 스포너를 꺼둬야 함 | 레이어 초기 `Unloaded`라 자동 |

**생성 흐름**

```text
Fixer 수락 → 트리거 진입 → StartMission → SetupSegment
  → DL_Mission: Unloaded → Activated
      → 셀 로드, 적·경로·키·문·칩·셔터 액터가 디스크 저장 상태로 생성
      → 각 적: BeginPlay
           → UMissionEnemyComponent::BeginPlay
                → AWarehouseMission::FindActive(this)      등록부에서 미션을 찾음
                → Mission->RegisterEnemy(this)
                     ++AliveEnemies
                     OnPawnDeath.AddDynamic(HandleEnemyDied)
           → AI 컨트롤러 빙의 → ST_NeonDistrictEnemy 시작 → 순찰
```

미션은 적이 몇 명인지 미리 모른다. 레이어가 올라오면서 적들이 스스로 등록하고, 그 수가 곧 "이 구간의 적 수"다.
적을 더 놓거나 빼도 코드가 안 바뀐다.

**마지막 적 → 키**

```text
적 사망 → AShooterNPC::Die → OnPawnDeath 방송
  → AWarehouseMission::HandleEnemyDied     (Fighting 단계일 때만 셈)
       --AliveEnemies
       0 이면: "Dead" 태그가 붙은 적을 찾아 그 위치에 KeyClass 스폰 → AdvanceTo(KeyDropped)
```

`OnPawnDeath`는 인자가 없어서 "누가 죽었는지"를 안 준다. 템플릿이 죽을 때 `Dead` 태그를 붙이니 그걸로 마지막 적의 위치를 찾는다.
키는 **런타임 스폰**이라 레이어 밖 — 유일하게 미션이 직접 챙기는 액터다 (`DroppedKey`, `SetupSegment`에서 파괴).

**리셋 흐름**

```text
플레이어 사망 → HandlePlayerDied → SetupSegment
  → AliveEnemies = 0, DroppedKey 파괴, Step = Fighting
  → DL_Mission: Unloaded → (GC) → Activated
      → 살아 있던 적·죽은 적 시체 전부 사라짐 (옛 델리게이트 구독도 같이)
      → 새 적들이 BeginPlay에서 다시 등록 → 다시 N명
```

`AliveEnemies`를 0으로 맞추는 이유: 레이어가 내려갈 때 옛 적들의 `EndPlay`는 돌지만 `OnPawnDeath`는 안 나가서, 새 적들이 세는 값이
옛 값 위에 쌓이지 않게. 옛 적의 구독은 옛 적 객체와 함께 사라진다.

**배치 규칙.** 적·경로 전부 `DL_Mission`에 — 밖이면 미션 시작 전에 `BeginPlay`가 돌아 등록에 실패하고, 수락 전부터 보이고, 죽어도 안 돌아온다.
`BP_NeonDistrictEnemy`는 `BP_ShooterNPC`의 **자식**(재부모 아님, 13-7). 미션 액터·트리거·부활 지점·Fixer는 레이어 **밖**.
시체는 템플릿 `DeferredDestructionTime`(5초) 뒤 자동 파괴.

**걸렸던 것**

| 문제 | 원인 | 해결 |
|---|---|---|
| `[Warehouse] 적 등록` 로그가 안 찍힘 | 재부모한 BP가 핫 리로드로 생긴 임시 클래스 `ANeonDistrictEnemy`에 물림 (`Default__ANeonDistrictEnemy`) | 에디터 재시작 후 다시 부모 지정 → 결국 컴포넌트 방식으로 |
| 키를 안 줍고 죽으면 키가 남음 | 스폰된 키는 레이어 밖 | `SetupSegment`에서 `DroppedKey` 파괴 |
| 키 주운 뒤 프롬프트 잔류 | 파괴된 대상을 약한 포인터가 스스로 `nullptr`로 만들어 "변화 없음"으로 판정 | `IsStale()`도 변화로 (8절) |

### 13-2. 상태 전환 — Patrol → Investigate → Attack (템플릿이 이미 하는 것)

로드맵엔 Patrol → Investigate → Chase → Combat 네 상태로 적었지만, 실제 트리는 **세 상태 + 전역 감지 태스크**다.
"Chase"는 별도 상태가 아니라 Attack 안에서 목표를 향해 이동하는 것이 곧 추격이다. 순찰만 우리가 만들었고(13-3~), 감지·조사·공격은
템플릿 것을 `ST_Shooter` 복제본 안에서 그대로 쓴다. 로드맵 5번의 "Investigate 태스크"와 "시야/소리 자극에 따른 상태 전이"는
새로 만들 게 아니라 확인만 하면 되는 항목이었고, 순찰 검증 때 "보면 추격·사격, 놓치면 조사 후 복귀"로 확인됐다.

```text
                 ┌──────────── Sense Enemies (전역, 항상 돎) ────────────┐
                 │  OnSeeEnemy  /  OnInvestigateLocation  /  OnForgetEnemy │
                 └───────┬──────────────┬──────────────────┬──────────────┘
                         ▼              ▼                  ▼
Search for Enemy   ──────────►  Attack Enemy  ◄────────  Investigating
 (Patrol)          OnSeeEnemy   (Chase+Combat)  OnSeeEnemy   (Investigate)
   ▲                              │                            ▲
   │        OnForgetEnemy ────────┘                            │
   └───────────────── 조사 끝, 못 찾음 ────────────────────────┘
                     OnInvestigateLocation: Patrol → Investigating
```

| 상태 (트리 이름) | 로드맵 이름 | 안에서 하는 일 | 나가는 조건 |
|---|---|---|---|
| `Search for Enemy` | Patrol | `Next Patrol Point` → `Move To` → `Delay 2±1` → 반복 | `OnSeeEnemy` → Attack / `OnInvestigateLocation` → Investigating |
| `Investigating` | Investigate | `Move to Investigate Location`(자극 위치로) → 도착 후 두리번 → 완료 | 완료 → Patrol / `OnSeeEnemy` → Attack |
| `Attack Enemy` | Chase + Combat | 진입 조건 `Is Object Valid(TargetActor)`. 병렬로 `Face Towards Actor` + `Shoot at Target`, 자식 상태로 `Find Sniping Location`(EQS, 목표에서 500~2000cm) → `Move to Sniping Location` → `Wait` → 반복 | `OnForgetEnemy` → Patrol |
| `Dead` | — | `Current HP <= 0`이면 최우선. `Delay 1`, 트리 종료 | — |

**Chase가 따로 없는 이유.** `Find Sniping Location`이 "목표로부터 Min~Max 거리의 사격 위치"를 EQS로 고르고 거기로 걷는다.
목표가 멀면 다가가고 가까우면 거리를 벌린다. 걷는 동안에도 `Shoot at Target`이 병렬로 돌아 쏘면서 이동한다.
따로 나누면 오히려 "쫓는 동안은 안 쏨"이 된다.

**전환을 일으키는 것 — `Sense Enemies` 전역 태스크.** `AIPerception`(시야·청각) 자극이 `AShooterAIController::OnShooterPerceptionUpdated`
→ `FStateTreeSenseEnemiesTask`로 넘어오고, 태스크가 자극의 종류에 따라 세 델리게이트 중 하나를 방송한다. 상태 전환은 이 델리게이트를 트리거로 걸려 있다.

| 자극 | 판정 | 방송 | 결과 |
|---|---|---|---|
| `Player` 태그 액터를 **직접 봄** — 정면 85° 콘 안 + 라인 트레이스 무차단 | `bDirectLOS == true` | `OnSeeEnemy` + `TargetActor` 설정 | → Attack |
| 봤지만 콘 밖 / 가려짐, 또는 소리 | 부분 감지. 이미 목표가 있으면 무시 | `OnInvestigateLocation` + `InvestigateLocation` = 자극 위치 | → Investigating |
| 자극이 `Max Age` 넘게 안 옴 | `OnPerceptionForgotten` | `OnForgetEnemy` + 목표 해제 | → Patrol |

부분 감지는 강도가 시간에 따라 감쇠한 이전 자극보다 클 때만 새 조사를 시작한다 (`ScaledStimulus = LastStrength / max(경과, 1)`).
발소리 하나하나에 매번 반응하지 않게 하는 장치.

**조정할 수 있는 손잡이 (필요해지면).** 감지 범위·각도는 `BP_NeonDistrictAIController`의 `AIPerception` Sight 설정 덮어쓰기(골목이 좁으니 거리는 줄이고
각도는 넓히는 쪽). 놓친 뒤 기억 시간은 Sight `Max Age`. 교전 거리는 트리 파라미터 `SnipingMinDistance / MaxDistance`(500 / 2000, 실내는 줄여야 함).
순찰 대기는 `Idle at Roam Location`의 Delay. 템플릿 총성이 Hearing 자극을 내는지는 아직 확인 안 함 — 안 나면 `UAISense_Hearing::ReportNoiseEvent`를
무기 발사에 걸어야 소리로도 Investigate가 시작된다.

관련: 템플릿 `ShooterStateTreeUtility.h/.cpp`(`FStateTreeSenseEnemiesTask`, `FStateTreeLineOfSightToTargetCondition`, `FStateTreeShootAtTargetTask`), `ShooterAIController.h`.

### 13-3. 순찰 — 로밍을 지정 경로로

템플릿 적은 EQS로 임의 지점을 골라 돌아다닌다(로밍). 우리는 지정 경로를 순서대로 돌게 바꾼다.
바뀌는 건 "지점을 고르는 태스크" 하나이고, 이동·대기·조사·추격·전투는 템플릿 트리를 그대로 쓴다.
관련 커밋: `b47be2d`(적 배치·키 드롭), `5f19453`(순찰). 배치 절차는 `docs/enemy_placement.md`.

```text
템플릿:  Roam(EQS 임의 지점)  → Investigate → Attack
우리:    Patrol(지정 경로)     → Investigate → Attack       ← 첫 칸만 교체
```

### 13-4. 조각별 역할

| 조각 | 역할 |
|---|---|
| `APatrolRoute` | 점 배열(액터 기준 상대 좌표, `MakeEditWidget`으로 뷰포트에서 끌기), `bLoop`. `GetPointWorld` / `NextIndex`(순환·왕복) / `NearestIndex`. **상태 없음** — 여러 적이 공유 |
| `UMissionEnemyComponent` | 적에 붙는 컴포넌트. `PatrolRoute` 참조(인스턴스 지정) + 순찰 진행 `PatrolIndex` · `PatrolDirection` + 미션 등록 |
| `FStateTreeNextPatrolPointTask` | C++ StateTree 태스크. 컨텍스트 액터 → 컴포넌트 → 경로. 다음 점을 골라 트리 파라미터 `TargetMovement_Location`에 써 넣고 즉시 `Succeeded` |
| `ST_NeonDistrictEnemy` | `ST_Shooter` 복제본. `Find Roam Location` 상태의 EQS 태스크만 우리 태스크로 교체 |
| `BP_NeonDistrictAIController` | `BP_ShooterAIController` 자식. StateTreeAI 컴포넌트의 트리만 우리 것 |
| `BP_NeonDistrictEnemy` | `BP_ShooterNPC` 자식. AI Controller Class를 우리 컨트롤러로. `Mission Enemy` 컴포넌트 부착 |

### 13-5. 트리 안에서의 흐름

```text
Search for Enemy
 ├ Find Roam Location     Next Patrol Point → Parameters.TargetMovement_Location    (성공 → 다음 / 실패 → Idle)
 ├ Move to Roam Location  Move To ← Parameters.TargetMovement_Location
 └ Idle at Roam Location  Delay 2±1 → Root → 다시 처음
```

템플릿은 세 상태가 **트리 파라미터**로 값을 주고받는다 — 상태가 다르면 서로의 태스크에 직접 바인딩할 수 없어서다.
그래서 우리 태스크도 값을 "내놓는" 게 아니라 EQS 태스크와 같은 형식으로 파라미터에 **쓴다**:
`TStateTreePropertyRef<FVector>`(Category `Out`) + `GetMutablePtr(Context)`. 트리 구조는 손대지 않고 태스크 한 줄만 갈아 끼우면
`Move To`가 그대로 새 값을 읽는다.

### 13-6. 설계 판단

**순찰 진행은 태스크가 아니라 컴포넌트에.** StateTree 태스크의 인스턴스 데이터는 **상태에 들어올 때마다 새로 만들어진다** —
나가면 버려지고 다시 들어오면 에셋 기본값이다. 처음엔 `CurrentIndex`를 태스크에 뒀다가 매번 "지점 1"만 고르는 걸 로그로 보고 알았다.
"다음이 어디냐"는 적 자신의 기억이라 컴포넌트가 든다. 죽어서 재생성되면 컴포넌트도 새로 생겨 `-1`로 돌아오니 레이어 리셋과 같이 간다.

**경로도 컴포넌트가 든다.** 트리의 Context Actor Class는 `BP_ShooterNPC`라 우리 BP에 변수를 만들어도 트리가 못 본다.
태스크가 컨텍스트 액터에서 `FindComponentByClass`로 컴포넌트를 찾아 읽으면 트리·템플릿 BP를 안 건드린다.

**첫 진입은 가장 가까운 점부터.** 같은 경로를 도는 적 둘이 모두 0번으로 가면 한 점에 몰린다. `PatrolIndex < 0`이면 `NearestIndex`.

**경로 액터는 상태가 없다.** 적 여럿이 한 경로를 다른 위치·방향에서 돌 수 있어야 하니, "몇 번째·어느 방향"은 적 쪽이다.

**경로가 없으면 `Failed`.** 트리의 실패 전환(→ Idle)으로 빠져 트리가 멈추지 않는다.
실패 세 갈래(액터 없음 / 경로 없음 / 바인딩 안 됨)는 로그로 남겨 설정 실수를 바로 잡는다.

**템플릿 자산 무수정.** 복제·자식 BP만. 재부모는 안 된다 — `ST_Shooter`가 `BP_ShooterNPC_C`의 BP 속성에 직접 바인딩돼 있어
자식이 아닌 클래스가 컨텍스트에 오면 런타임에 단언으로 죽는다 (13-7).

### 13-7. 걸렸던 것

| 문제 | 원인 | 해결 |
|---|---|---|
| 적 BP를 재부모했더니 크래시 `PropertyBindingBindingCollection.cpp:1273` | 트리 바인딩이 `BP_ShooterNPC_C` 속성을 참조. 자식이 아니면 `IsChildOf` 단언. 컴파일은 통과하고 런타임에 터진다 | C++ 서브클래스 대신 **컴포넌트** + Child Blueprint |
| Context Actor Class 드롭다운에 `ShooterNPC` 없음 | 클래스 선택창이 `abstract` 클래스를 숨김 (`AllowAbstract` 메타 없으면) | 컨텍스트 클래스는 `BP_ShooterNPC` 그대로 두고 자식으로 통과 |
| 링크 오류 `FPropertyBindingBindingCollection::GetAddress` | 프로퍼티 참조 **쓰기**가 `PropertyBindingUtils` 모듈 필요 (템플릿은 읽기만) | `Build.cs`에 추가 |
| 매번 "지점 1" | 태스크 인스턴스 데이터가 상태 진입마다 초기화 | 진행을 컴포넌트로 이동 |
| `경로 없음(컴포넌트 없음)` — 컴포넌트가 분명 있는데 | 에디터 켠 채 빌드 → 컴포넌트 클래스 재생성 → 기존 인스턴스는 옛 클래스. `FindComponentByClass`가 새 클래스로 검사해 못 찾음 | 에디터 끄고 번호 DLL 정리 후 빌드 |
| 점이 `X=5280` — 2초마다 제자리 | NavMesh 밖이면 Move To 즉시 실패 → Idle 반복 | `P` 키로 확인, 경로를 초록 위로 |
| `APatrolRoute();56702186` | 키 입력 사고. 뒤 선언까지 연쇄 오류(`NumPoints`가 멤버가 아니다) | 숫자 삭제 |
| 에디터를 닫았는데 `Unable to delete hot-reload file` | 프로세스가 뒤에서 종료 중 | 작업 관리자에서 `UnrealEditor.exe` 확인 |

### 13-8. 이번에 쓴 API

| | 뜻 |
|---|---|
| `meta = (MakeEditWidget)` | `FVector`(배열)에 뷰포트 드래그 위젯 |
| `FStateTreeTaskCommonBase` + 인스턴스 데이터 `USTRUCT` | C++ StateTree 태스크의 두 구조체. `GetInstanceDataType`으로 연결 |
| `Category = Context` / `Out` | 컨텍스트 자동 바인딩 / 쓸 곳을 바인딩으로 받는 출력 |
| `TStateTreePropertyRef<T>` + `GetMutablePtr(Context)` | 바인딩된 프로퍼티(트리 파라미터)에 쓰기 |
| `bShouldCallTick = false` | 진입 시 한 번만 일하는 태스크 |
| `EStateTreeRunStatus::Succeeded / Failed` | 상태 전환 트리거 |
| `Transient` | 런타임 상태. 저장 안 됨 |
| `DECLARE_DYNAMIC_MULTICAST_DELEGATE` + `AddDynamic` + `UFUNCTION()` 핸들러 | 템플릿 `OnPawnDeath`. 블루프린트용 델리게이트라 우리 `AddUObject`와 다르다 |
| Create Child Blueprint Class | 템플릿 BP를 안 건드리고 설정 하나만 덮어쓰기 |

### 13-9. 검증

수락 → 진입 → `[Warehouse] 적 등록 - 2명` → 적 둘이 각자 `지점 0 → 1 → 2 → 0 …` 순서로 걷고 점마다 멈춤 →
플레이어를 보면 추격·사격, 놓치면 조사 후 복귀 → 다 잡으면 마지막 적 자리에 키 드롭 → 콘솔 없이 완주.
