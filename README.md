# Neon District

> Unreal Engine 5로 제작하는 사이버펑크 FPS 퀘스트 버티컬 슬라이스

**Neon District**는 밀도 높은 도시 구역 하나에서 탐색, NPC 대화, 퀘스트 진행, 전투, 목표물 회수, 컷씬까지 이어지는 완결된 플레이 흐름을 구현하는 포트폴리오 프로젝트입니다.

플레이 가능한 공간은 작고 정교하게 구성하고, 실제로 이동할 수 없는 원경 도시는 최적화된 메시와 라이팅으로 표현하여 규모감과 성능을 함께 확보하는 것을 목표로 합니다.

> 현재 상태: 기획 및 프로토타이핑

## Gameplay Flow

```text
도시 거리 탐색
    ↓
Fixer NPC와 대화 및 퀘스트 수락
    ↓
적이 점거한 건물에 침입
    ↓
적 AI와 FPS 전투
    ↓
데이터 칩 획득
    ↓
Fixer NPC에게 복귀
    ↓
Level Sequence 종료 컷씬
```

## Core Features

### Environment

- 약 `150m × 150m` 규모의 플레이 구역
- 네온사인, 상점 골목, 전투 골목, 2층 규모의 적 건물
- Skyline, 고층 빌딩, 안개를 활용한 사이버펑크 도시 원경
- Emissive Material과 Lumen을 활용한 야간 라이팅

### FPS Combat

- 무기 발사 및 재장전
- 체력과 피격 처리
- 적 명중 판정과 전투 피드백
- 건물 내·외부를 연결하는 전투 공간

### Enemy AI

```text
Patrol → Investigate → Chase → Combat
```

- 지정 경로 순찰
- 소리 또는 플레이어 감지
- 추격 및 사격
- 전투 상황에 따른 상태 전환

### Quest & Interaction

- 상호작용 안내 UI
- NPC 대화 및 퀘스트 수락
- 퀘스트 목표 갱신
- 목표물 획득 및 NPC 복귀 판정

### Cinematics

- Sequencer 기반 시작 및 종료 컷씬
- 시네마틱 카메라 연출
- NPC 애니메이션과 대화 연동

## World Optimization

플레이 영역 밖의 도시는 실제 공간을 모두 제작하지 않고, 실루엣과 조명 중심의 원경으로 구성합니다.

- World Partition 기반 구역 스트리밍
- HLOD를 활용한 원거리 Static Mesh 통합
- 반복 건물의 Instanced Static Mesh 구성
- 거리별 LOD와 Cull Distance 적용
- 원경용 단순화 메시 및 저해상도 텍스처 사용
- 불필요한 내부 구조와 Collision 제거

### 측정과 판단

고정 카메라 5곳에서 동일한 그래픽 설정으로 측정합니다 (Standalone 1920×1080, RTX 3060 Ti / Ryzen 5 5600X).
아트 패스를 마친 현재 상태의 수치는 다음과 같습니다.

| 카메라 | Frame (ms) | GPU (ms) | FPS |
|---|---:|---:|---:|
| 01 StreetEntrance | 14.35 | 13.37 | 69.7 |
| 02 AlleyInterior | 12.88 | 11.91 | 77.6 |
| 03 Hideout2FWindow | 13.24 | 12.29 | 75.5 |
| 04 CombatZone | 14.15 | 13.16 | 70.7 |
| 05 SkylineOverlook | 13.84 | 12.90 | 72.3 |

이 프로젝트에서 최적화는 **목표치가 아니라 측정 결과에 따라 판단한 항목**입니다.
다섯 카메라가 12.9~14.4ms 안에 모여 있고, 건물이 시야를 가득 채운 카메라와 거의 비어 있는 카메라의
차이가 1.5ms뿐입니다. 즉 이 씬의 비용은 콘텐츠가 아니라 Lumen · VSM · Substrate의 고정 비용이
지배하므로, 프랍을 줄이거나 LOD를 손보는 작업은 세션 편차(±0.5ms) 수준의 변화만 만듭니다.

실제로 확인한 두 가지:

- 건물 콜리전을 convex hull 528개 → 13개로 단순화 — 프레임 변화 측정되지 않음
  (이 작업의 가치는 프레임이 아니라 물리 안정성과 에디터 작업성이었습니다)
- 메쉬 12개에 Nanite 활성화 — 프레임 변화 ±0.6ms(노이즈 범위),
  매 프레임 발생하던 VSM `Non-Nanite Marking Job Queue overflow` 경고는 완전히 해소

목표인 60 FPS를 모든 카메라에서 충족하므로, 남은 일정은 추가 최적화 대신 버그 수정과
완주 품질에 사용했습니다. 측정 절차와 전후 데이터, 측정 중 겪은 함정은
[docs/perf/](docs/perf/)에 기록되어 있습니다.

## Level Structure

```text
Neon District
├─ City Street
├─ Market Alley
├─ Fixer NPC Area
├─ Combat Alley
└─ Gang Hideout
   ├─ 1F Combat Area
   └─ 2F Objective Area
```

## Development Roadmap

- [ ] 플레이어 이동 및 기본 FPS 시스템
- [ ] 무기 발사, 재장전, 체력 및 피격 처리
- [ ] Greybox 레벨과 전투 동선 제작
- [ ] 적 AI 상태 및 전투 행동 구현
- [ ] NPC 상호작용과 대화 시스템
- [ ] 퀘스트 수락, 갱신 및 완료 흐름 구현
- [ ] 데이터 칩 목표물과 획득 UI 구현
- [ ] 도시 환경 및 네온 라이팅 제작
- [ ] World Partition, LOD, HLOD 최적화
- [ ] 시작 및 종료 컷씬 제작
- [x] 성능 측정과 최적화 전후 비교
- [ ] 최종 플레이 테스트 및 폴리싱

## Documentation

작업 규칙과 최적화 기록은 `docs/` 에 정리합니다.

| 문서 | 내용 |
|---|---|
| [docs/roadmap.md](docs/roadmap.md) | 구현 로드맵 — 현재 상태, 작업 순서와 의존, 항목별 작업 체크리스트 |
| [docs/story.md](docs/story.md) | 스토리 — 전제, 비트 시퀀스, Fixer 대사, 엔딩 연출 |
| [docs/known_issues.md](docs/known_issues.md) | 알려진 문제와 버그 기록 — 열린 문제, 미완 기능, 닫힌 문제의 실제 원인 |
| [docs/enemy_placement.md](docs/enemy_placement.md) | 적·순찰 경로 배치 가이드 — 절차, 계획 배치, 자주 걸리는 것 |
| [docs/optimization_intro.md](docs/optimization_intro.md) | 원경 최적화 입문 — 드로우 콜, 링 구조, HLOD를 처음부터 설명하는 해설서 |
| [docs/level_workflow.md](docs/level_workflow.md) | 링 레이어 구조, 폴더 규칙, 맵 제작 및 원경 최적화 워크플로우 |
| [docs/level_flow.md](docs/level_flow.md) | 플레이 동선 설계, 구역 좌표, 시야 설계 |
| [docs/level_blockout_shots.md](docs/level_blockout_shots.md) | 블록아웃 구역별 스크린샷과 검토 기록 |
| [docs/perf/README.md](docs/perf/README.md) | 성능 측정 프로토콜과 콘솔 명령 |
| [docs/perf/baseline.md](docs/perf/baseline.md) | 최적화 기준선 측정 기록 |
| [docs/perf/01_nanite_overhead.md](docs/perf/01_nanite_overhead.md) | Nanite 구성 전환 비용, 세션 간 재현성 확인 |
| [docs/perf/02_art_pass.md](docs/perf/02_art_pass.md) | 아트 패스 이후 기준선, 메쉬 12개 Nanite 활성화, 측정 함정 |
| [docs/perf/nanite_experiment.md](docs/perf/nanite_experiment.md) | Nanite vs 전통 LOD 비교 실험 설계 |

작업 맵: `Content/NeonDistrict/Maps/Lvl_NeonDistrict`

## Tools

- Unreal Engine 5.8
- ComfyUI — 콘셉트 이미지 및 시각 자료 제작
- Git / GitHub — 버전 관리 및 개발 기록

## 외부 에셋과 저장소 범위

레벨은 Fab에서 받은 에셋 팩에 의존하고, **그 에셋들은 저장소에 포함되지 않습니다.**
재배포 권한이 없고 용량이 크기 때문입니다 (`Content/cyberdistrict/` 외 팩들, `Content/MyAssets/` 261MB).

| 저장소에 있는 것 | 저장소에 없는 것 |
|---|---|
| C++ 소스 전체, 블루프린트, 레벨과 액터, 머티리얼 인스턴스, 데이터 레이어·HLOD 설정, 문서 | Fab 메쉬·텍스처 원본 (`cyberdistrict`, `Fab`, `NeonGrid`, `EasyFog`, `MyAssets` 등) |

따라서 **새로 클론해서 `Lvl_NeonDistrict`를 열면 프롭과 건물 메쉬가 비어 있습니다.**
저장소는 구현과 작업 기록을 담고, 레벨의 시각적 재현은 보장하지 않습니다.
플레이 가능한 결과물은 패키징된 빌드와 스크린샷·영상으로 확인합니다.

제외된 애셋은 [`Scripts/Backup-Assets.ps1`](Scripts/Backup-Assets.ps1)로 외부 위치에 미러합니다
(robocopy 증분, 기본 2.2GB). 이 선택의 비용도 기록해 둡니다 — 메쉬 애셋에 한 작업(콜리전 형상, Nanite 설정)은
버전 관리되지 않습니다. 10/14에 프롭 17종의 콜리전을 수정한 작업이 그 예로,
저장소에는 남지 않았습니다. 자세한 내용은 [docs/known_issues.md](docs/known_issues.md) B-02에 있습니다.

## Project Goals

- 탐색부터 퀘스트 완료까지 이어지는 하나의 완성된 게임 루프 구현
- 전투, AI, UI, 컷씬을 하나의 플레이 경험으로 통합
- 제한된 공간이 넓은 도시처럼 느껴지는 레벨 디자인 연구
- 수치로 검증할 수 있는 환경 최적화 과정 기록
- Unreal Engine 기반 게임플레이 및 Technical Art 역량 제시

## Disclaimer

이 프로젝트는 학습과 포트폴리오 제작을 위한 비상업적 개인 프로젝트입니다. 특정 상용 게임의 에셋, 캐릭터, 로고 또는 서사를 사용하지 않으며, 모든 프로젝트 리소스는 직접 제작하거나 사용 권한이 있는 자료로 구성할 예정입니다.

