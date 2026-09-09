# Neon District — C++ 코드 정리

이 프로젝트 고유의 게임플레이 C++ 코드(`Source/CyberPunkProject/*/NeonDistrict/`)가
무엇을 하고, 왜 그렇게 짰는지 정리한 문서다.

기준 시점: 작업 트리(커밋 전) 상태. 비교 대상 커밋은 `4556b5b feat: add a C++ weapon class carrying a static mesh gun`.

---

## 1. 전체 그림

이 프로젝트의 게임플레이는 UE5 **Shooter 템플릿**(`Variant_Shooter/`)에서 출발한다.
템플릿 원본은 **수정하지 않고 참고 자료로 남긴다**는 것이 `docs/roadmap.md` 1번 항목의 방침이고,
NeonDistrict 코드는 그 템플릿을 **상속으로 덮어쓰는 층**이다.

```text
AGameModeBase
 └─ AShooterGameMode        (템플릿, UCLASS(abstract) — 직접 쓸 수 없다)
     └─ ANeonDistrictGameMode   ← 프로젝트 전용 게임모드

AActor
 └─ AShooterWeapon          (템플릿 — 발사/재장전/탄약 로직 전부 보유)
     └─ ANeonDistrictWeapon     ← 프로젝트 전용 무기(외형 + 에셋 바인딩만 담당)
```

| 파일 | 역할 |
|---|---|
| `Public/NeonDistrict/NeonDistrictGameMode.h` | 게임모드 선언. 시작 무기 클래스 프로퍼티 + 오버라이드 2개 |
| `Private/NeonDistrict/NeonDistrictGameMode.cpp` | 폰·컨트롤러·UI 클래스 지정, 스폰 직후 시작 무기 지급 |
| `Public/NeonDistrict/NeonDistrictWeapon.h` | 무기 선언. 스태틱 메시 총 컴포넌트 |
| `Private/NeonDistrict/NeonDistrictWeapon.cpp` | 총 메시·투사체·애님 BP 에셋 바인딩 |

---

## 2. `ANeonDistrictGameMode`

### 목적

`AShooterGameMode`는 `UCLASS(abstract)`라 레벨에 직접 지정할 수 없다.
그래서 **구체 클래스가 하나 필요했고**, 동시에 로드맵 1번의 "시작 무기 지급"을 여기에 붙였다.
지금 이 클래스가 하는 일은 두 가지다.

1. 어떤 폰·컨트롤러·UI를 쓸지 정한다 (`InitGame`)
2. 플레이어가 스폰되면 총을 쥐어준다 (`HandleStartingNewPlayer_Implementation`)

### 멤버

```cpp
UPROPERTY(EditDefaultsOnly, Category="Neon District")
TSubclassOf<AShooterWeapon> StartingWeaponClass;
```

- 생성자에서 `ANeonDistrictWeapon::StaticClass()`로 기본값을 넣는다.
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

### `HandleStartingNewPlayer_Implementation()` — 왜 다음 틱인가

```cpp
Super::HandleStartingNewPlayer_Implementation(NewPlayer);   // 부모가 폰 스폰 + 빙의

TWeakObjectPtr<APlayerController> WeakPC(NewPlayer);
GetWorldTimerManager().SetTimerForNextTick([this, WeakPC]()
{
    if (!WeakPC.IsValid()) return;
    if (IShooterWeaponHolder* Holder = Cast<IShooterWeaponHolder>(WeakPC->GetPawn()))
    {
        Holder->AddWeaponClass(StartingWeaponClass);
    }
});
```

핵심 세 가지.

- **`Super`가 먼저** — 부모가 폰을 스폰하고 빙의시키기 전에는 `GetPawn()`이 null이다.
- **다음 틱으로 미룸** — 빙의 직후 프레임에는 폰의 메시/애님 초기화가 아직 진행 중이다.
  그 시점에 무기를 붙이면 `AttachWeaponMeshes`가 소켓을 못 찾거나 애님 인스턴스 교체가 씹힐 수 있다.
  한 틱 뒤로 미뤄 초기화가 끝난 상태를 보장한다.
- **`TWeakObjectPtr`** — 람다가 다음 틱에 실행되는 사이에 컨트롤러가 파괴될 수 있다.
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
| `ANeonDistrictCharacter` 생성 | **미착수** — 지금은 템플릿 `BP_ShooterCharacter`를 그대로 쓴다 |
| 리스폰 비활성화 / 체크포인트 재시작 | **미착수** — `ShooterCharacter`의 `RespawnTime = 5.0f`가 그대로 살아 있다 |
| 시작 무기 지급 (`AddWeaponClass`) | 완료 |
| `Lvl_NeonDistrict`에 GameMode Override | 완료 (커밋 전) |
| `PlayerStart` (-6600, 0, 120) 확인 | 미확인 |

---

## 6. 정리해두면 좋을 자잘한 것

기능에는 영향이 없지만 다음에 손댈 때 같이 치울 것들이다.

- `NeonDistrictGameMode.cpp`의 `#include "UObject/ConstructorHelpers.h"` — 이제 `ConstructorHelpers`를 쓰지 않으므로 불필요하다.
- 같은 파일 `ShooterUI.h` include 옆 주석이 `// ← FClassFinder<APawn>`으로 잘못 붙어 있다. 실제로는 `ShooterUIClass` 타입 때문에 필요한 include다.
- `NeonDistrictGameMode.h`에서 `InitGame` 위에 `HandleStartingNewPlayer`용 주석("플레이어가 스폰되어 폰에 빙의한 직후 호출된다")이 중복으로 붙어 있다.
- `NeonDistrictWeapon.cpp`의 `#include "Components/SkeletalMeshComponent.h"` — 직접 쓰는 곳이 없다(`GetFirstPersonMesh()`의 반환 타입 때문인데 헤더 체인으로 이미 들어온다).
- `GunMesh`의 `SetRelativeLocation(0)` / `SetRelativeRotation(0)`은 기본값과 같아 실질적인 동작이 없다. 총구 정렬을 잡을 때 쓸 자리 표시로 남겨둔 것이라면 그대로 두어도 된다.
