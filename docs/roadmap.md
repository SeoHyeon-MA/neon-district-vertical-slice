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

### 미착수

`Content/NeonDistrict/` 아래에 `Blueprints` 폴더가 없다. 이 프로젝트 고유의 게임플레이 에셋은 아직 하나도 없으며,
현재 동작하는 게임플레이는 전부 UE5 템플릿이다.

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

현 시점에 `Lvl_NeonDistrict`에서 PIE를 실행하면 무기도 적도 없는 맨몸 1인칭으로 블록아웃을 걸어다니게 된다.

---

## 2. 작업 순서

```text
[1] GameMode ──┬──> [3] 퀘스트 상태 머신 ──┬──> [4] Fixer 대화 ──┐
               │                          │                    ├──> [7] UI ──> [8] 컷씬
[2] 상호작용 ──┴──────────────────────────┴──> [6] 칩·셔터 ─────┘
               │
               └──> [5] 적 AI 재설계

[9] 최적화 측정, [10] Variant_Horror 제거, [11] CP77_Study 분리 — 독립 진행
```

| # | 항목 | 우선순위 | 의존 |
|---|---|---|---|
| 1 | 전용 GameMode 신설과 아레나 규칙 제거 | P0 | – |
| 2 | 상호작용 프레임워크 | P0 | – |
| 3 | 퀘스트 상태 머신 서브시스템 | P0 | 1 |
| 4 | Fixer NPC 대화와 퀘스트 수락 | P1 | 2, 3 |
| 5 | 적 AI 재설계 | P1 | 1 |
| 6 | 데이터칩 목표물과 셔터 개방 | P1 | 2, 3 |
| 7 | HUD·UI 세트 구성 | P2 | 2, 3, 4, 6 |
| 8 | 시작·종료 시네마틱 | P2 | 3, 4 |
| 9 | 최적화 전후 측정 | P2 | – |
| 10 | Variant_Horror 제거 | P2 | – |
| 11 | CP77_Study 쿠킹 분리 | P2 | – |

### 1일차 범위

**1, 2, 3번.** 목표는 완성된 기능이 아니라 *끝에서 끝까지 통과하는 배선*이다.
이 셋이 서면 이후 모든 기능이 붙을 자리가 생긴다.

---

## 3. 항목 상세

### 1. 전용 GameMode 신설과 아레나 규칙 제거

**우선순위** P0 · **의존** 없음

템플릿의 데스매치 규칙을 걷어내고 프로젝트 전용 게임모드를 세운다.
템플릿 원본은 수정하지 않고 상속으로 덮어, 원본을 참고 자료로 남긴다.

- [ ] `Source/CyberPunkProject/NeonDistrict/` 폴더 신설
- [ ] `ANeonDistrictGameMode` (`AShooterGameMode` 파생), `ANeonDistrictCharacter` (`AShooterCharacter` 파생) 생성
- [ ] 플레이어 리스폰 비활성화, 사망 시 체크포인트 재시작 처리
- [ ] 시작 무기 지급 (`AddWeaponClass`)
- [ ] `Lvl_NeonDistrict`에 GameMode Override 지정
- [ ] `PlayerStart` (-6600, 0, 120) 위치 확인

**완료 기준** — PIE 실행 시 무기를 든 상태로 시작해 A 진입부부터 F 적 건물까지 완주할 수 있고, 사망해도 리스폰이 아니라 재시작된다.

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

### 3. 퀘스트 상태 머신 서브시스템

**우선순위** P0 · **의존** 1

슬라이스의 척추. 셔터 개방, HUD 목표 갱신, 종료 컷씬 트리거가 전부 이 상태를 구독한다.
이게 없으면 각 요소가 서로를 직접 참조하면서 의존이 얽힌다.

```text
NotStarted → Accepted → ChipAcquired → ShutterOpen → Returned → Complete
```

- [ ] `UQuestSubsystem` (GameInstanceSubsystem) — 상태 enum, `AdvanceTo()`, 현재 목표 텍스트
- [ ] `OnQuestStateChanged` 멀티캐스트 델리게이트 (블루프린트 바인딩 가능)
- [ ] 화면 좌상단 목표 트래커 위젯이 델리게이트 구독
- [ ] 콘솔 치트 커맨드 `ND.SetQuestState <n>` — 다른 기능 없이도 전 구간 테스트 가능하게

**완료 기준** — 콘솔로 상태를 강제 전환하면 HUD 목표 텍스트가 따라 바뀐다. 이후 모든 기능은 이 델리게이트에만 연결한다.

---

### 4. Fixer NPC 대화와 퀘스트 수락

**우선순위** P1 · **의존** 2, 3

B 대로 `MRK_Fixer` (-3000, 600)의 NPC와 대화해 퀘스트를 수락한다.
비트 시퀀스 2단계이자, 10단계 복귀 시 재사용된다.

- [ ] Fixer NPC 액터 — `IInteractable` 구현, 적대 AI와 분리된 클래스
- [ ] 대사 데이터 테이블 (화자, 본문, 다음 노드, 퀘스트 상태 조건)
- [ ] 대화 위젯 — 진행/종료, 대화 중 이동 입력 잠금
- [ ] 대화 종료 시 `QuestSubsystem`을 `Accepted`로 전이
- [ ] 퀘스트 상태에 따른 대사 분기 (수락 전 / 진행 중 / 복귀)

**완료 기준** — 대로에서 Fixer에게 말을 걸면 대사가 진행되고, 종료 시 HUD 목표가 갱신된다. 칩 획득 후 다시 말을 걸면 다른 대사가 나온다.

---

### 5. 적 AI 재설계 — Patrol → Investigate → Chase → Combat

**우선순위** P1 · **의존** 1

사격·조준·시야 판정(`FStateTreeLineOfSightToTargetCondition`, `ST_Shooter_ShootAtTarget`)은 그대로 재사용하고,
로밍 대신 순찰과 조사 단계를 추가한다.

- [ ] **월드 파티션 환경에서 블록아웃 전역에 NavMesh가 깔리는지 먼저 검증** — 막히면 이 항목 전체가 멈춘다
- [ ] 순찰 경로 액터 (포인트 배열 또는 스플라인), 레벨 배치
- [ ] Patrol StateTree 태스크 — 경로 순회, 지점 대기
- [ ] Investigate 태스크 — 마지막 인지 위치로 이동, 탐색 후 미발견 시 Patrol 복귀
- [ ] 시야/소리 자극에 따른 상태 전이 조건 정리 (`AIPerception` 델리게이트는 기존 것 사용)
- [ ] 스포너 리스폰 비활성화, 적 5명 고정 배치 (D 골목 2, E 광장 1, F 1F 1, F 2F 1)

**완료 기준** — 적이 지정 경로를 순찰하다 플레이어를 감지하면 추격·교전하고, 놓치면 마지막 위치를 조사한 뒤 순찰로 복귀한다. 사망한 적은 리스폰하지 않는다.

---

### 6. 데이터칩 목표물과 셔터 개방

**우선순위** P1 · **의존** 2, 3

비트 시퀀스 8~9단계. `docs/level_flow.md`의 "복귀는 되돌아가기가 아니라 지름길 개방" 원칙을 구현한다.
셔터는 `X 500~2500` 구간, 높이 600cm.

- [ ] 데이터칩 픽업 액터 — `IInteractable` 구현, F 2F 목표 구역 배치
- [ ] 획득 시 `QuestSubsystem`을 `ChipAcquired`로 전이
- [ ] 획득 알림 UI + 사운드·이펙트 자리 확보
- [ ] 셔터 액터 — `OnQuestStateChanged` 구독, `ChipAcquired`에서 개방 애니메이션 재생
- [ ] 개방 후 상태를 `ShutterOpen`으로 전이
- [ ] 복귀 동선 실제 통행 검증 (셔터 통과 → 대로 → Fixer)

**완료 기준** — 칩을 집으면 알림이 뜨고 목표가 "Fixer에게 복귀"로 바뀌며, 대로 동쪽 셔터가 열려 우회 없이 직선 복귀할 수 있다.

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
- [ ] 종료 시퀀스 — `QuestSubsystem` `Returned` 상태 구독하여 재생
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

- [ ] 레퍼런스 뷰어로 `Variant_Horror` 에셋을 참조하는 다른 에셋이 없는지 확인
- [ ] `Content/Variant_Horror/` 에디터에서 삭제
- [ ] `Source/CyberPunkProject/Variant_Horror/` 삭제 후 프로젝트 리빌드
- [ ] `Config/`에 남은 Horror 관련 참조 정리
- [ ] 빌드 및 PIE 정상 동작 확인

**완료 기준** — Variant_Horror 관련 코드와 에셋이 모두 제거된 상태에서 프로젝트가 정상 빌드되고 `Lvl_NeonDistrict`가 실행된다.

---

### 11. CP77_Study 학습 자료 쿠킹 대상에서 분리

**우선순위** P2 · **의존** 없음 (첫 패키징 전까지)

`Content/CP77_Study/`는 상용 게임 환경을 분석해 재구성 기법을 익히기 위한 학습 자료다(`docs/CP77_Study/`).
학습 기록으로서 가치가 있으므로 저장소에는 남기되, 최종 빌드에서는 분리한다.
README의 Disclaimer가 밝힌 대로 이 프로젝트의 리소스는 직접 제작하거나 사용 권한이 있는 자료로 구성하며,
학습용 임포트 자산이 배포물에 섞이면 안 된다.

- [ ] `Project Settings > Packaging`의 `Directories to never cook`에 `/Game/CP77_Study` 추가
- [ ] 슬라이스 레벨이 CP77_Study 에셋을 참조하지 않는지 레퍼런스 확인
- [ ] `Content/NewCubeLevel.umap` (임시 테스트 맵) 유지·삭제 결정
- [ ] `Content/Developers/` 쿠킹 제외 확인
- [ ] `SourceArt/`, `Tools/` 저장소 포함 여부와 `.gitignore` 방침 결정
- [ ] 학습 자료임을 `docs/CP77_Study/README_KO.md`에 명시

**완료 기준** — 패키징 결과물에 학습용 임포트 자산이 포함되지 않고, 저장소에는 학습 기록이 그대로 남는다.
