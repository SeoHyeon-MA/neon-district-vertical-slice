# Neon District — 구현 로드맵

기획(`README.md`)과 레벨 설계(`docs/level_flow.md`)를 실제 구현 작업 단위로 분해한 문서다.
각 항목은 배경, 작업 체크리스트, 완료 기준, 선행 의존을 갖는다.

---

## 1. 현재 상태

### 완료된 것

| 영역 | 내용 | 근거 |
|---|---|---|
| 레벨 | A~F 전 구역 블록아웃, 동선·시야 설계 확정 | `docs/level_flow.md` |
| 원경 | 링 구조 배치, HLOD 레이어, 데이터 레이어 | `Content/NeonDistrict/` |
| 측정 | 성능 카메라 5개, 베이스라인, Nanite 실험 설계 | `docs/perf/` |
| FPS 기본 | 이동·사격·재장전·체력·피격·무기 픽업·탄약 HUD | `Source/CyberPunkProject/Variant_Shooter/` |
| AI 기반 | AIPerception, StateTree, EQS, 시야 판정 조건 | `Variant_Shooter/AI/` |

### 게임플레이 진행 (2026-09-14 기준)

9/7~9/14 사이에 붙은 것. 상세는 `docs/neon_district_code.md`.

| 영역 | 내용 | 커밋 |
|---|---|---|
| 게임모드 | `ANeonDistrictGameMode` — 폰·컨트롤러·UI 지정, 스폰마다 무기 지급, 체크포인트 재시작 | `0b31f8b` `bb1e60f` |
| 캐릭터 | `ANeonDistrictCharacter` — 템플릿 5초 리스폰 취소, 지연 후 재시작, `OnDied` 방송. `BP_NeonDistrictCharacter` | `bb1e60f` |
| 무기 | `ANeonDistrictWeapon`(abstract) + `Pistol`/`Rifle` — 스태틱 메시 총, 종류별 애님·모델·오프셋·연사 | `4556b5b` `30ec023` `1016ce9` |
| 미션 | `ANeonDistrictMission` — 진입 트리거, `EMissionState`, `DeathCount`, 델리게이트 구독, `SetupSegment` 루프 | `d302fec` `eea6986` `ece1d88` |
| 정리 | Variant_Horror 제거, CP77_Study 쿠킹 제외, Public/Private 구조 | `8cea04a` `e78f25b` |

아직 없는 것: 상호작용 프레임워크, NPC 대화, 적 배치·순찰, 창고 키·아이템, HUD, 컷씬, 환경 아트.

### 핵심 격차

템플릿은 **아레나 데스매치**이고, 기획은 **선형 퀘스트 버티컬 슬라이스**다.
이 장르 차이를 메우는 것이 초기 작업의 실체다.

| 템플릿 동작 | 근거 | 기획에 맞는 동작 |
|---|---|---|
| 사망 후 5초 리스폰 | `ShooterCharacter.h` `RespawnTime = 5.0f` | 체크포인트 재시작 |
| 스포너가 적을 무한 리스폰 | `ShooterNPCSpawner.h` | 배치형 고정 5명 |
| 팀 스코어 집계 | `ShooterGameMode::IncrementTeamScore` | 퀘스트 상태로 대체 |
| EQS 무작위 로밍 | `EQS_FindRoamLocation` | 지정 경로 순찰 |
| 시작 게임모드가 FirstPerson 템플릿 | `Config/DefaultEngine.ini` `GlobalDefaultGameMode` | 프로젝트 전용 게임모드 |

(2026-09-07 시점의 진단. 리스폰·시작 게임모드·팀 스코어는 해결됐고, 적 무한 리스폰과 EQS 로밍은 5번 항목에서 다룬다.)

---

## 2. 작업 순서

```text
[1] GameMode ✓ ─┬──> [3] 미션 상태·전파 ──┬──> [4] Fixer 대화 ──┐
                │         (뼈대 ✓)         │                    ├──> [7] UI ──> [8] 컷씬
[2] 상호작용 ───┴──────────────────────────┴──> [6] 창고 키·아이템 ┘
                │
                └──> [5] 적 AI 재설계 ──> 미션 SetupSegment 에 스폰 연결

[9] 최적화 측정 ── 아트(10/5~) 이후
[10] Variant_Horror 제거 ✓   [11] CP77_Study 분리 ✓
```

| # | 항목 | 우선순위 | 의존 | 상태 (9/14) |
|---|---|---|---|---|
| 1 | 전용 GameMode 신설과 아레나 규칙 제거 | P0 | – | **완료** |
| 2 | 상호작용 프레임워크 | P0 | – | 미착수 |
| 3 | 미션 상태 머신과 상태 전파 | P0 | 1 | 뼈대 완료 — 3-A 결정됨, 구현 중 |
| 4 | Fixer NPC 대화와 미션 수락 | P1 | 2, 3 | 미착수 |
| 5 | 적 AI 재설계 | P1 | 1, 3 | 미착수 |
| 6 | 창고 키·창고·아이템, 셔터 개방 | P1 | 2, 3, 5 | 미착수 |
| 7 | HUD·UI 세트 구성 | P2 | 2, 3, 4, 6 | 미착수 |
| 8 | 시작·종료 시네마틱 | P2 | 3, 4 | 미착수 |
| 9 | 최적화 전후 측정 | P2 | 아트 | 미착수 |
| 10 | Variant_Horror 제거 | P2 | – | **완료** |
| 11 | CP77_Study 쿠킹 분리 | P2 | – | **완료** |

주간 배정은 4절.

---

## 3. 항목 상세

### 1. 전용 GameMode 신설과 아레나 규칙 제거

**우선순위** P0 · **의존** 없음

템플릿의 데스매치 규칙을 걷어내고 프로젝트 전용 게임모드를 세운다.
템플릿 원본은 수정하지 않고 상속으로 덮어, 원본을 참고 자료로 남긴다.

- [x] `Source/CyberPunkProject/{Public,Private}/NeonDistrict/` 폴더 신설
- [x] `ANeonDistrictGameMode`, `ANeonDistrictCharacter` 생성
- [x] 플레이어 리스폰 비활성화, 사망 시 체크포인트 재시작 처리 — `Die()` 오버라이드, `RestartPlayer` 체크포인트 분기
- [x] 시작 무기 지급 — `SetPlayerDefaults`에서 스폰마다 `StartingWeapons` 지급
- [x] `Lvl_NeonDistrict`에 GameMode Override 지정
- [x] `PlayerStart` 위치 확인

**완료 기준** — PIE 실행 시 무기를 든 상태로 시작해 A 진입부부터 F 적 건물까지 완주할 수 있고, 사망해도 리스폰이 아니라 재시작된다. **충족 (9/10).**

템플릿 원본을 건드린 곳은 `ShooterCharacter.h`의 `virtual` 두 개뿐이다 (`b2a77e4`).

---

### 2. 상호작용 프레임워크

**우선순위** P0 · **의존** 없음

Fixer 대화, 데이터칩 획득, 셔터 개방, 문 여닫기가 모두 같은 배선에 붙어야 한다.
각 액터가 개별적으로 입력을 처리하면 이후에 얽힌다.

- [ ] `IInteractable` 인터페이스 — `GetInteractionPrompt()`, `Interact(APawn*)`, `CanInteract()`
- [ ] `UInteractionComponent` — 플레이어에 부착, 전방 트레이스로 대상 탐색 및 갱신
- [ ] 상호작용 입력 액션 추가 (Enhanced Input, `Content/Input/Actions`)
- [ ] 프롬프트 위젯 — `[E] 대화` 형태, 대상이 있을 때만 표시
- [ ] `Content/LevelPrototyping/Interactable/Door`로 동작 검증

**완료 기준** — 블록아웃에 배치한 문에 접근하면 프롬프트가 뜨고 키를 누르면 열린다. 대상에서 벗어나면 프롬프트가 사라진다.

---

### 3. 미션 상태 머신과 상태 전파

**우선순위** P0 · **의존** 1

슬라이스의 척추. 창고 문, HUD 목표 갱신, NPC 대사 분기, 종료 컷씬이 전부 이 상태를 구독한다.

**9/14 현재** — 원래 계획은 `UQuestSubsystem`(GameInstance 서브시스템)이었으나, 9/10~9/14에 기획이
"미션을 받고 구역에 들어가면 시작, 죽어도 진행 중 유지, 사망 횟수로 랭크"로 구체화되면서
레벨에 배치하는 `ANeonDistrictMission` 액터가 그 역할의 절반을 이미 맡았다 (`ece1d88`).

```text
EMissionState:  NotAccepted → Accepted → InProgress → Completed
```

- [x] 진입 트리거, 수락 게이트, `InProgress` 유지, `DeathCount`
- [x] 캐릭터 `OnDied` / 게임모드 `OnPlayerPawnReady` 델리게이트 구독, 부활 후 재구독
- [x] `SetupSegment()` — 시작과 재시도가 같은 함수
- [x] 콘솔 `ND.AcceptMission`

**3-A. 결정 (9/14) — 공통 뼈대와 미션별 세부를 분리하고, 소비자는 질의 함수만 쓴다.**

`InProgress` 안의 세부 단계(적 처리 → 키 → 창고 → 아이템 → 복귀)를 어디에 둘지가 문제였다.
한 열거형에 다 넣으면(A안) 미션 하나일 땐 단순하지만, 두 번째 미션부터 세부 단계가 섞인다.
세부 단계는 미션마다 다르고 큰 흐름(수락 전/수락/진행/완료)·사망 횟수·재시도·랭크는 모든 미션이 같으므로,
**공통은 부모에, 세부는 자식에** 둔다. 그리고 HUD·문·NPC가 열거형 값을 직접 보면 미션이 늘 때마다 같이 고쳐야 하므로
**소비자는 `GetObjectiveText()` 같은 질의 함수만** 쓴다. 분리와 질의 둘 다 있어야 확장이 된다.

```text
ANeonDistrictMission (abstract, 공통 뼈대)
  State, DeathCount, 진입 트리거, RestartPoint, OnDied/OnPlayerPawnReady 구독,
  StartMission → SetupSegment(), HandlePlayerDied → SetupSegment(), CompleteMission → 랭크
  virtual GetObjectiveText()    virtual SetupSegment()    OnStateChanged 방송

  └─ AWarehouseMission (이번 슬라이스)
       EStep { Fighting → KeyDropped → KeyAcquired → ItemAcquired }   (9/18: Returning 제거, KeyAcquired 추가)
       GetObjectiveText() override — Step별 문구
       SetupSegment() override — 적 정리·생성, Step = Fighting, 키·창고 원상복구

소비자 (HUD, 창고 문, 셔터, Fixer) → Mission->GetObjectiveText() 등 질의만. 열거형 직접 참조 금지
```

원칙 둘. **부모는 미션이 뭔지 모른다** — 언제 시작하고 죽으면 어떻게 되고 몇 번 죽었는지만 안다.
**소비자는 미션 내부를 모른다** — 지금 목표가 뭔지 묻고, 바뀌었다는 방송만 듣는다.
두 번째 미션은 만들지 않는다. 만들 수 있는 구조라는 것만 보이면 된다.

- [x] 부모: `SetupSegment()`·`GetObjectiveText()`를 `virtual`로, `UCLASS(abstract)`로. `IsInProgress()`, `GetRank()` 질의 (`c27b387`)
- [x] 부모: `OnStateChanged` 멀티캐스트 델리게이트, `NotifyStateChanged()` 한 곳에서 방송
- [x] `AWarehouseMission` 생성 — `EWarehouseStep`, 두 오버라이드. 레벨의 미션 액터 교체
- [ ] 콘솔 `ND.SetMissionStep <n>` — 다른 기능 없이도 단계 강제 전환
- [ ] 소비자가 미션을 찾는 통로 — `UWorldSubsystem`에 현재 미션 등록. 게임모드는 미션을 모른다는 원칙 유지
- [ ] HUD 목표 문구 위젯이 `OnStateChanged` 구독 → `GetObjectiveText()` 표시
- [ ] `CompleteMission()` — `DeathCount`로 랭크 (0회 S / 1회 A / 2회 B / 3회↑ C)

**완료 기준** — 콘솔로 단계를 강제 전환하면 HUD 목표 문구가 따라 바뀐다. 이후 모든 기능은 델리게이트와 질의 함수에만 연결한다.

**3-B. 결정 (9/16) — 구간 초기화는 런타임 데이터 레이어 재활성화로.**

`SetupSegment()`를 적 정리·키 회수·문 잠금 코드로 채우는 대신, 적·키·창고 문을 **`DL_Mission` 런타임 데이터 레이어**에 넣고
`Unloaded → Activated`로 전환한다. 레이어 안의 액터가 전부 에디터 저장 상태로 재로드되므로 수동 리셋 코드가 없고,
되돌릴 대상이 늘어도 누락이 생기지 않는다.

"미션 구역을 별도 레벨로 운영하는 게 나았을까"라는 질문에서 나온 결론이다. 별도 레벨의 이점(재로드 = 전체 초기화)은 맞지만,
초기화되면 안 되는 것(`State`, `DeathCount`)은 어차피 레벨 밖에 있어야 하므로 미션 액터는 그대로 필요하다.
두 방식은 대립하지 않고, 달라지는 건 `SetupSegment()`의 구현뿐이다. World Partition에서 서브레벨에 해당하는 단위가
런타임 데이터 레이어라 별도 레벨 파일도 필요 없다.

부수 효과: 수락 전에는 레이어를 `Unloaded`로 두면 "미션 미수락 시 적 없음"이 코드 없이 충족된다. 적은 스포너 대신 에디터 직접 배치.

미션 액터·트리거·부활 지점은 절대 이 레이어에 넣지 않는다.

**구현 결과 (9/17) — "내렸다 올리기"는 두 단계 대기가 필요했다.**

처음 예상한 위험은 "활성화가 비동기라 완료를 기다려야 한다"였는데, 실제로 걸린 건 다른 두 가지였다.

1. **같은 프레임에 내렸다 올리면 아무 일도 안 일어난다.** `SetDataLayerRuntimeState(Unloaded)`는 셀 스트리밍 레벨에
   플래그만 켜고 실제 제거는 다음 스트리밍 업데이트가 한다. 그 전에 `Activated`가 오면 플래그만 도로 뒤집힌다.
2. **내려간 레벨이 GC 전이면 엔진이 그 레벨을 그대로 재사용한다.** `WorldPartitionLevelStreamingDynamic.cpp`의
   "Reuse existing Level" — 셀 경계를 왔다갔다할 때 디스크를 다시 읽지 않으려는 최적화다. 일반 레벨 스트리밍은 내려간 뒤
   GC를 강제하지만(`GLevelStreamingForceGCAfterLevelStreamedOut`), World Partition은 셀이 많아 히치를 피하려고 이걸 끈다
   (`WorldPartitionSubsystem.cpp`). 그래서 기본 GC 주기(약 60초) 안에 올리면 밀린 큐브가 밀린 자리 그대로 돌아왔다.
   콘솔 명령으로 됐던 건 두 명령 사이에 우연히 GC가 돌았던 것.

해결: `ANeonDistrictMainMission::SetupSegment()`가 내리기 전에 레이어 셀의 레벨 패키지 이름을 기억해 두고, 타이머(0.1초)로
① 셀이 월드에서 빠졌는지 → ② 그 패키지가 메모리에서 사라졌는지(`StaticFindObjectFast` + `Garbage` 제외) 순서로 확인한다.
②가 아직이면 `GEngine->ForceGarbageCollection(true)`로 다음 틱 GC를 요청하고 다시 기다린다. 둘 다 통과하면 `Activated`.
World Partition이 끈 GC를 이 레이어에 한해 우리가 대신 거는 셈이다. 전체 리셋에 약 0.2초, 부활 대기 2초 안에 끝난다.
전역 콘솔 변수 `LevelStreaming.ShouldReuseUnloadedButStillAroundLevels 0`으로 재사용을 끄는 방법도 있지만, GC 전까지
셀이 안 올라와 최대 60초 빈 구간이 생기고 모든 셀의 스트리밍 성능에 영향을 줘서 택하지 않았다.

검증은 `-NDSegmentTest` 인자로 혼자 도는 임시 테스트(레이어 올리기 → 큐브 밀기 → `SetupSegment()` → 0.05초마다 상태 덤프)를
`-game -unattended`로 돌려 로그로 했다. 재활성화 전후 액터 이름이 같다는 것이 재사용의 증거였다. 확인 후 테스트 코드는 제거.

레이어 리셋은 창고만의 일이 아니라 메인 미션 공통이라 `AWarehouseMission`이 아닌 `ANeonDistrictMainMission`에 두었다.

- [x] `DL_Mission` 런타임 데이터 레이어 생성, 초기 상태 `Unloaded`
- [x] `ANeonDistrictMainMission`에 `SegmentLayer` 프로퍼티, `SetupSegment()`에서 `UDataLayerManager`로 재활성화
- [x] 로드 완료 시점 확인 — 완료 델리게이트가 아니라 "월드에서 빠짐 → GC로 사라짐" 두 단계 폴링

---

### 4. Fixer NPC 대화와 미션 수락

**우선순위** P1 · **의존** 2, 3

B 대로 `MRK_Fixer` (-3000, 600)의 NPC와 대화해 퀘스트를 수락한다.
비트 시퀀스 2단계이자, 10단계 복귀 시 재사용된다.

- [x] Fixer NPC 액터 — `IInteractable` 구현, 적대 AI와 분리된 클래스 (`979498e`)
- [x] 대사 데이터 테이블 (화자, 본문, 다음 행, Effect) (`a6d19ca`)
- [x] 대화 위젯 — 한 줄씩 진행/종료, 이동·시점 잠금 (`b7df929`), 무기 입력 잠금 (`74eb05c`)
- [x] 대화 종료 시 미션 `AcceptMission()` 호출 — 코드가 아니라 행의 `Effect`가 시점을 정한다 (`a6d19ca`)
- [x] 미션 상태에 따른 대사 분기 + 복귀 시 `CompleteMission()` (`6ca09ad`)
- [x] 대화 중 NPC 카메라로 전환 (`16e3e08`) — 각도는 NPC가 든다
- [ ] 기획서의 "전화로 미션을 받는" 연출 — **통화 화면 겉모습**(프레임·노이즈·신호 아이콘)은 `WBP_Dialogue`의 문제라 7번(UI)으로 옮긴다. 카메라 연출은 위에서 끝났다
- [ ] 대사를 `docs/story.md`의 표로 교체 (`Intro_4`, `Return_2~3` 추가)

**취소 — 미션 완료 후 NPC를 종료 위치로 이동** (기획서 "npc 위치 옮기기", 9/23 결정)

세계가 진행 상태를 반영하는 흔한 장치이고 구현도 간단하지만(`OnStateChanged` 구독 → `Completed`에 지정 위치로),
**이 슬라이스에서는 플레이어가 볼 수 없다.** 완료 직후 화면이 옥상 엔딩 시퀀스로 넘어가고(`docs/story.md`),
미션이 하나뿐이라 "다음 미션을 위한 재배치"도 의미가 없다. 보이지 않는 기능을 만드는 대신 항목에서 뺀다.

정말 필요해지면 — 완료 직후 Fixer가 부스를 떠나는 것을 짧게 보여주고 페이드하는 연출 — 그때는 게임플레이 코드가 아니라
8번 컷씬의 시퀀서 안에서 처리한다. 그 경우 이 항목 자체가 다시 생기지 않는다.

**완료 기준** — 대로에서 Fixer에게 말을 걸면 카메라가 전환되며 대사가 진행되고, 종료 시 HUD 목표가 갱신된다.
칩 획득 후 다시 말을 걸면 다른 대사가 나오고 미션이 완료되며 랭크가 뜬다. **9/22 충족.**

---

### 5. 적 AI 재설계 — Patrol → Investigate → Chase → Combat

**우선순위** P1 · **의존** 1, 3

사격·조준·시야 판정(`FStateTreeLineOfSightToTargetCondition`, `ST_Shooter_ShootAtTarget`)은 그대로 재사용하고,
로밍 대신 순찰과 조사 단계를 추가한다.

- [ ] **월드 파티션 환경에서 블록아웃 전역에 NavMesh가 깔리는지 먼저 검증** — 막히면 이 항목 전체가 멈춘다
- [ ] 순찰 경로 액터 (포인트 배열 또는 스플라인), 레벨 배치
- [ ] Patrol StateTree 태스크 — 경로 순회, 지점 대기
- [ ] Investigate 태스크 — 마지막 인지 위치로 이동, 탐색 후 미발견 시 Patrol 복귀
- [ ] 시야/소리 자극에 따른 상태 전이 조건 정리 (`AIPerception` 델리게이트는 기존 것 사용)
- [ ] 스포너 리스폰 비활성화, 적 5명 고정 배치 (D 골목 2, E 광장 1, F 1F 1, F 2F 1)
- [ ] **적 5명을 `DL_Mission` 레이어에 직접 배치** — 스포너 대신. 미션 시작·사망 시 `SetupSegment()`의 레이어 재활성화로 초기화된다 (3-B). 미수락 시 레이어 `Unloaded`라 적 없음
- [ ] 마지막 적 사망 시 창고 키 드롭 (6번과 맞물림)

**완료 기준** — 적이 지정 경로를 순찰하다 플레이어를 감지하면 추격·교전하고, 놓치면 마지막 위치를 조사한 뒤 순찰로 복귀한다. 사망한 적은 리스폰하지 않는다.

---

### 6. 창고 키·창고·아이템, 셔터 개방

**우선순위** P1 · **의존** 2, 3, 5

비트 시퀀스 8~9단계. 9/10 기획으로 "데이터칩을 줍는다"가 **"마지막 적이 창고 키를 떨어뜨리고, 키로 창고를 열어 아이템을 얻는다"** 로 바뀌었다.
`docs/level_flow.md`의 "복귀는 되돌아가기가 아니라 지름길 개방"(셔터, `X 500~2500`, 높이 600cm)은 그대로 유지한다.

- [ ] 창고 키 픽업 액터 — 마지막 적 사망 위치에 스폰, `IInteractable`
- [ ] 창고 문 액터 — 키 없으면 "잠김" 프롬프트, 있으면 개방
- [ ] 아이템(데이터칩) 픽업 — 창고 안, 획득 시 미션 상태 `ItemAcquired`
- [ ] 획득 알림 UI + 사운드·이펙트 자리 확보
- [ ] 셔터 액터 — 미션 `OnStateChanged` 구독, `ItemAcquired`에서 개방
- [ ] 키·창고 문을 `DL_Mission` 레이어에 넣고, 사망 시 레이어 재활성화로 원상복구되는지 확인 (3-B)
- [ ] 복귀 동선 실제 통행 검증 (셔터 통과 → 대로 → Fixer)

**완료 기준** — 마지막 적을 잡으면 키가 떨어지고, 키로 창고를 열어 아이템을 얻으면 목표가 "Fixer에게 복귀"로 바뀌며 셔터가 열린다.

---

### 7. HUD·UI 세트 구성

**우선순위** P2 · **의존** 2, 3, 4, 6

현재 HUD는 템플릿의 탄약 카운터(`UI_ShooterBulletCounter`)뿐이다.

- [ ] 체력 표시
- [ ] 목표 트래커 시각 정리 (뼈대는 3번에서 생성)
- [ ] 상호작용 프롬프트 시각 정리 (뼈대는 2번에서 생성)
- [ ] 대화창 스타일 통일
- [ ] 획득 알림
- [ ] 사망 · 재시작 화면
- [ ] 크로스헤어와 피격 피드백
- [ ] 사이버펑크 톤(네온, 스캔라인) 일괄 적용

**완료 기준** — 전 구간 플레이 동안 목표·체력·탄약·상호작용 가능 여부를 화면만 보고 파악할 수 있다.

---

### 8. 시작·종료 시네마틱

**우선순위** P2 · **의존** 3, 4

비트 시퀀스 1단계(A 진입부 스케일 압도)와 10단계(복귀 후 종료).
기존 `SEQ_LevelFlow_Flythrough`는 레벨 검토용 플라이스루라 그대로 쓸 수 없다.

- [ ] 시작 시퀀스 — A 진입부에서 원경 랜드마크 `DIST_R1_00` (거리 522m, 앙각 18.4°) 프레이밍
- [ ] 종료 시퀀스 — 미션 `Completed` 전이 시 재생. 기획서대로 "마지막 왔던 거리를 돌아보는" 카메라
- [ ] 미션 완료 창 (랭크 표시) → 게임 종료
- [ ] 재생 중 입력 잠금 및 종료 후 컨트롤 반환
- [ ] 시네마틱 카메라 세팅 (피사계 심도, 노출)
- [ ] NPC 애니메이션과 대사 타이밍 연동
- [ ] 스킵 지원

**완료 기준** — 게임 시작과 퀘스트 완료 시 컷씬이 재생되고, 끝나면 조작이 정상 복귀한다.

---

### 9. 최적화 전후 측정과 README 표 채우기

**우선순위** P2 · **의존** 없음 (아트 패스 이후)

포트폴리오의 핵심 근거. 측정 카메라 `CAM_Perf_01~05`와 프로토콜은 이미 준비돼 있고,
측정 지점은 연출 의도와 일치하도록 설계됐다 — A 진입부(완전 개방), F 2F 서향 창(실내에서 원경, 최악의 렌더링 조합).

- [ ] 아트 패스 이후 최적화 전 상태 재측정
- [ ] World Partition 스트리밍, HLOD, ISM, LOD/Cull Distance 적용
- [ ] 동일 카메라·동일 그래픽 설정으로 After 측정
- [ ] `docs/perf/`에 기록, README 표(Average FPS / GPU Frame Time / Draw Calls / Visible Primitives) 채우기
- [ ] 비교 스크린샷 정리

**완료 기준** — README의 최적화 전후 표가 실측치로 채워지고, 각 수치의 근거 문서가 `docs/perf/`에 남는다.

---

### 10. Variant_Horror 코드·에셋 제거

**우선순위** P2 · **의존** 없음

UE5 First Person 템플릿에 딸려 온 호러 변형 샘플로, 이 기획에서 쓰이지 않는다.
쿠킹 용량과 저장소 탐색성만 해친다.

제거 대상: `Source/CyberPunkProject/Variant_Horror/`, `Content/Variant_Horror/`,
`Content/__ExternalActors__/Variant_Horror/`, `Content/__ExternalObjects__/Variant_Horror/`

- [x] `Variant_Horror` 에셋을 참조하는 다른 에셋이 없는지 확인 — 외부 참조 0건
- [x] `Content/Variant_Horror/`, `__ExternalActors__`, `__ExternalObjects__` 삭제 (100개 에셋)
- [x] `Source/CyberPunkProject/Variant_Horror/` 삭제 (8개 파일)
- [x] `CyberPunkProject.Build.cs`의 `Variant_Horror` 인클루드 경로 2개 제거
- [x] `Config/`에 남은 Horror 관련 참조 정리 — 원래 없었음
- [x] 빌드 확인 — `Result: Succeeded` (2026-09-07)
- [ ] 에디터에서 PIE 정상 동작 확인

**완료 기준** — Variant_Horror 관련 코드와 에셋이 모두 제거된 상태에서 프로젝트가 정상 빌드되고 `Lvl_NeonDistrict`가 실행된다.

> **빌드가 한 번 막혔던 기록** — 이 삭제와 무관한 엔진 측 문제였다.
> 엔진 룰 어셈블리 캐시 `UE_5.8/Engine/Intermediate/Build/BuildRules/UE5Rules.dll`은 8/31 생성인데,
> Rider가 9/4에 `RiderLink` 플러그인을 엔진에 설치하면서 그 안의 `RD` 모듈이 캐시에 없는 상태가 됐고,
> UBT가 `Expecting to find a type ... named 'RD'` (RulesError)로 중단했다.
>
> **주의 — 이 캐시는 그냥 지우면 안 된다.** 이 엔진은 설치형(binary) 빌드라
> (`Engine/Build/InstalledBuild.txt` 존재) UBT가 룰 어셈블리를 읽기 전용으로 취급해 자동 재생성하지 않는다.
> 지우면 `Precompiled rules assembly ... does not exist`로 바뀌며, `-ForceRulesCompile`로도 풀리지 않는다.
>
> 재생성 방법: `InstalledBuild.txt`를 잠시 옮겨 설치형 판정을 끈 상태에서
> `UnrealBuildTool.dll -Mode=QueryTargets -Project=<uproject>`를 실행하면 C++ 컴파일 없이 캐시만 다시 만들어진다.
> 실행 직후 표식 파일을 반드시 원위치시킨다. 확실한 대안은 Epic Games Launcher의 UE 5.8 파일 검증이다.

---

### 11. CP77_Study 학습 자료 쿠킹 대상에서 분리

**우선순위** P2 · **의존** 없음 (첫 패키징 전까지)

`Content/CP77_Study/`는 상용 게임 환경을 분석해 재구성 기법을 익히기 위한 학습 자료다(`docs/CP77_Study/`).
학습 기록으로서 가치가 있으므로 저장소에는 남기되, 최종 빌드에서는 분리한다.
README의 Disclaimer가 밝힌 대로 이 프로젝트의 리소스는 직접 제작하거나 사용 권한이 있는 자료로 구성하며,
학습용 임포트 자산이 배포물에 섞이면 안 된다.

- [x] `Config/DefaultGame.ini`에 `DirectoriesToNeverCook` 항목으로 `/Game/CP77_Study` 추가
- [x] 슬라이스 레벨이 CP77_Study 에셋을 참조하지 않는지 레퍼런스 확인 — 참조 0건
- [x] `Content/Developers/`도 같은 방식으로 쿠킹 제외
- [ ] `Content/NewCubeLevel.umap` (임시 테스트 맵) 유지·삭제 결정 — git 미추적이라 삭제 시 복구 불가, 판단 필요
- [ ] `SourceArt/`, `Tools/` 저장소 포함 여부와 `.gitignore` 방침 결정
- [ ] 학습 자료임을 `docs/CP77_Study/README_KO.md`에 명시

**완료 기준** — 패키징 결과물에 학습용 임포트 자산이 포함되지 않고, 저장소에는 학습 기록이 그대로 남는다.

---

## 4. 일정 — 2026-09-14 ~ 10-15

3절 항목을 주 단위로 배정한 것이다. 원안은 9/14에 세웠고, **1주차에서 이미 끝난 항목(게임모드·캐릭터·시작 무기·리스폰)을 걷어내고 미션 액터·창고 키·랭크를 넣어 조정했다.**

### 최종 목표

> 약 8~12분 분량의 탐색 → NPC 통화·미션 수락 → 전투 → 창고 키·아이템 획득 → 복귀 → 종료 컷씬·랭크로 이어지는 완성형 사이버펑크 FPS Vertical Slice 제작, 그리고 환경 최적화 전후 수치 정리.

### 주간 배정

| 기간 | 핵심 목표 | 항목 | 세부 작업 | 주간 결과물 |
|---|---|---|---|---|
| **9/14 ~ 9/20** ✅ | 게임플레이 뼈대 완성 | 2, 3 (+4, 6 대부분) | ~~GameMode·Character·시작 무기·리스폰~~ (9/8~9/14 완료) / ~~`IInteractable` + `UInteractionComponent` + E키 프롬프트 UI~~ / ~~미션 세부 단계·`OnStateChanged`·`ND.SetMissionStep` (3-A)~~ / ~~등록부·HUD 목표 문구~~ / ~~`SetupSegment()` 데이터 레이어 재활성화 (3-B)~~ / **앞당김**: ~~Fixer NPC·대사 테이블·대화 UI·분기·복귀 완료 (4번 ①~④)~~, ~~키·문·아이템 (6번 ①~③)~~ | NPC 통화 → 수락 → 진입 → 키 → 창고 → 아이템 → 복귀 → 완료·랭크가 콘솔 없이 플레이로 이어짐. 콘솔 의존은 `KeyDropped` 하나(적 없음). 죽으면 구간 리셋 |
| **9/21 ~ 9/27** | 미션 흐름 마무리 + 적 AI 착수 | 4, 6 잔여, 5 일부 | 아이템 픽업 커밋 / 셔터 개방 / 복귀 동선 검증 / NPC 종료 위치 이동 / 통화 연출(카메라) / 대화 중 사격 차단 / **앞당김**: World Partition NavMesh 검증, 순찰 경로 액터, 적 `DL_Mission` 배치 | 셔터까지 포함한 완주. 3주차 위험(NavMesh) 조기 해소. 적이 들어오면 `KeyDropped` 콘솔 의존 제거 |
| **9/28 ~ 10/4** | 적 AI와 전투 | 5, 7 일부 | NavMesh 검증 / 순찰 경로 액터 / Patrol·Investigate 태스크 / 상태 전이 / 적 5명 `DL_Mission`에 직접 배치, 무한 리스폰 제거 / 체력·크로스헤어·피격 HUD | 8~12분 흐름 안에서 실제 전투. 죽으면 적이 리셋되고 사망 횟수가 랭크에 반영 |
| **10/5 ~ 10/11** | 환경 아트·컷씬 | 7 나머지, 8 | **Ring 0 히어로 지점 3곳 우선**(A 진입부, E 광장, F 2F 창) / 모듈러 키트로 나머지 / Ring 1·2 원경 보강 / Emissive 야간 라이팅 / 시작·종료 시퀀스 / 미션 완료 창 / UI 톤 통일 | 처음부터 끝까지 연출이 연결된 빌드. 스크린샷 3장이 나오는 수준 |
| **10/12 ~ 10/15** | 측정·패키징·문서 | 9 | 아트 적용 후 Before 재측정 / HLOD·ISM·LOD·Cull Distance / 원경 Collision 제거 / After 측정 / 패키징 테스트 / README 표 채우기, `docs/perf` 갱신 | 플레이 가능한 최종 빌드 + 최적화 Before/After + 문서 |

### 원안에서 고친 것

| 원안 | 조정 | 이유 |
|---|---|---|
| 1주차에 GameMode·Character·리스폰 | 제거 | 9/14 이전에 완료됨 (`bb1e60f`) |
| `QuestSubsystem` 신설 | 미션 액터 확장 (3-A) | 9/10 기획이 미션 단위 재시도·랭크로 구체화되면서 `ANeonDistrictMission`이 이미 그 역할을 맡음 |
| "데이터칩 획득 → 셔터" | 창고 키 → 창고 → 아이템 → 셔터 | 9/10 기획서 |
| 2주차에 랭크 없음 | 2주차 말에 `CompleteMission` + 랭크 | 사망 횟수 집계는 이미 있으므로 계산만 붙이면 됨 |
| 4주차 "블록아웃 전체를 아트로 교체" | Ring 0 히어로 3곳 우선, 나머지는 키트 반복 | 150m×150m 전 구역 교체는 1주에 불가능. 스크린샷이 나오는 지점부터 |
| 5주차 4일에 최적화+버그+패키징+문서 | 측정·패키징·문서로 한정. 버그 수정은 4주차까지 | 4일은 새 작업이 아니라 마무리 시간 |

### 위험

- **4주차(아트)가 가장 빡빡하다.** 3주차 전투가 밀리면 아트 주간이 통째로 사라진다. 3주차 금요일(10/2)까지 전투가 안 되면 아트 범위를 히어로 지점 2곳으로 줄인다.
- **NavMesh.** 월드 파티션 환경에서 블록아웃 전역에 안 깔리면 3주차 전체가 멈춘다. 5번 항목 첫 줄이 검증인 이유. 2주차 안에 미리 한 번 확인해둔다.
- **성능 기준선.** 9/2 측정은 원경 스텁 상태라 비교용이 아니다. 아트 적용 직후 Before를 다시 재고, After와 **같은 카메라·같은 설정**으로만 비교한다. 보여줄 지표는 Draw Call보다 **GPU Frame Time** — 현재 병목이 GPU이고 Lumen·VSM·안개가 화면 전체 비용이기 때문.
- **`Content/Fab/` 라이선스.** 소총 모델이 커밋에 없어 저장소를 새로 받으면 소총이 안 보인다. 4주차 전에 결정.

### 보고서용 요약

**9/14 ~ 9/20 — 핵심 게임플레이 시스템 기반 구축 (완료, 커밋 30건)**
계획한 2·3번 항목을 완료하고, 2주차 예정이던 4번(NPC 대화)과 6번(키·문·아이템)의 대부분을 앞당겨 구현했다.
NPC 대화·아이템·문이 공통으로 쓰는 상호작용 프레임워크(`IInteractable` + `UInteractionComponent` + 프롬프트 UI)를 만들고,
미션을 추상 기반 → 메인 미션 → 창고 미션의 계층으로 세우고 세부 단계와 상태 변경 이벤트를 두었다. 월드 서브시스템 등록부로
HUD·문·NPC가 미션을 찾고, 게임모드와 소비자는 미션 내부를 모른다. 사망 시 구간 초기화는 런타임 데이터 레이어 재활성화로
해결했으며, World Partition이 스트림 아웃 후 GC를 미루고 레벨을 재사용하는 동작을 자동 테스트로 밝혀 GC 대기를 넣었다.
Fixer NPC는 데이터 테이블 기반 대사를 한 줄씩 보여주고 미션 상태에 따라 분기하며, 수락·완료 시점은 데이터가 정한다.
결과: NPC 통화 → 수락 → 진입 → 키 → 창고 → 아이템 → 복귀 → 완료·랭크가 콘솔 명령 없이 플레이로 이어진다.
부활 후 프롬프트 소실(폰 `BeginPlay`가 빙의보다 먼저 도는 문제)을 HUD 위젯을 컨트롤러 소유로 옮겨 해결했다.
`docs/neon_district_code.md` 7~12절에 기록. 잔여: 셔터, NPC 이동, 통화 연출, 아이템 픽업 커밋.

**9/21 ~ 9/27 — 미션 흐름 마무리 및 적 AI 착수**
1주차에 앞당긴 NPC 대화·키·문·아이템에 셔터 개방과 복귀 동선 검증을 더해 미션 흐름을 마무리하고, NPC 종료 위치 이동과
통화 연출(카메라)을 붙인다. 남는 시간으로 3주차 항목을 앞당겨 World Partition 환경의 NavMesh를 먼저 검증하고 순찰 경로 액터와
적 배치를 시작한다. 적이 들어오면 마지막 콘솔 의존(`KeyDropped`)이 사라진다.

**9/28 ~ 10/4 — Enemy AI 및 FPS 전투 구현**
Shooter Template의 AI 기반 위에 Patrol·Investigate·Chase·Combat 상태를 구성하고, 전투 구역에 적을 고정 배치한다. 미션 시작 시 적이 생성되고 사망 시 구간이 초기화되도록 미션 시스템과 연결하며, 사망 횟수가 랭크에 반영된다. 체력·크로스헤어·피격 효과 등 전투 HUD를 정리한다.

**10/5 ~ 10/11 — 환경 아트 및 시네마틱 제작**
플레이 구역의 핵심 지점부터 실제 환경 에셋으로 교체하고, 모듈러 키트와 네온 라이팅으로 사이버펑크 도시 분위기를 구성한다. 원경은 Ring 구조에 따라 상세도를 구분한다. 시작과 미션 완료 시 Sequencer 기반 컷씬을 추가하고 UI 스타일을 통일한다.

**10/12 ~ 10/15 — 최적화 측정 및 최종 정리**
환경 아트 적용 이후 성능 기준선을 다시 측정하고 World Partition·HLOD·Instancing·LOD·Cull Distance로 원경을 최적화한다. 동일한 측정 카메라와 설정으로 최적화 전후 GPU Frame Time을 비교하고, 패키징 테스트 후 README와 성능 문서를 정리한다.
