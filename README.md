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

최적화 결과는 동일한 카메라 위치와 그래픽 설정을 기준으로 전후 데이터를 기록할 예정입니다.

| Metric | Before | After |
| --- | ---: | ---: |
| Average FPS | TBD | TBD |
| GPU Frame Time | TBD | TBD |
| Draw Calls | TBD | TBD |
| Visible Primitives | TBD | TBD |

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
- [ ] 성능 측정과 최적화 전후 비교
- [ ] 최종 플레이 테스트 및 폴리싱

## Tools

- Unreal Engine 5.8
- ComfyUI — 콘셉트 이미지 및 시각 자료 제작
- Git / GitHub — 버전 관리 및 개발 기록

## Project Goals

- 탐색부터 퀘스트 완료까지 이어지는 하나의 완성된 게임 루프 구현
- 전투, AI, UI, 컷씬을 하나의 플레이 경험으로 통합
- 제한된 공간이 넓은 도시처럼 느껴지는 레벨 디자인 연구
- 수치로 검증할 수 있는 환경 최적화 과정 기록
- Unreal Engine 기반 게임플레이 및 Technical Art 역량 제시

## Disclaimer

이 프로젝트는 학습과 포트폴리오 제작을 위한 비상업적 개인 프로젝트입니다. 특정 상용 게임의 에셋, 캐릭터, 로고 또는 서사를 사용하지 않으며, 모든 프로젝트 리소스는 직접 제작하거나 사용 권한이 있는 자료로 구성할 예정입니다.

