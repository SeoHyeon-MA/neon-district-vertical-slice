# 적·순찰 경로 배치 가이드

`BP_NeonDistrictEnemy`와 `PatrolRoute`를 레벨에 놓는 절차. 코드는 안 바뀌고 전부 에디터 작업이다.
배치 위치는 `docs/level_flow.md` 비트 시퀀스(D 2, E 1, F 1F 1, F 2F 1)를 따른다.

## 구조 한눈에

```text
PatrolRoute (DL_Mission)            점 배열. 액터 위치 기준 상대 좌표
   ▲ Patrol Route 참조
BP_NeonDistrictEnemy (DL_Mission)   BP_ShooterNPC 자식
   └ Mission Enemy 컴포넌트          미션 등록 + 경로 참조 + 순찰 진행(PatrolIndex)
   └ AI Controller Class = BP_NeonDistrictAIController → ST_NeonDistrictEnemy
        Find Roam Location: Next Patrol Point → TargetMovement_Location 파라미터
        Move to Roam Location → Idle at Roam Location → 반복
```

미션이 시작되면 레이어가 올라오고, 각 적의 컴포넌트가 `BeginPlay`에서 미션에 등록한다. 마지막 적이 죽으면 그 자리에 키가 떨어진다.
죽으면 레이어 재활성화로 적·경로가 전부 처음 상태로 돌아온다.

## 계획 배치

| 구역 | 좌표 (level_flow) | 적 | 경로 | 비고 |
|---|---|---|---|---|
| D 전투 골목 | X −2000~+2500, Y −4000~−800 | 2 | `PR_Alley` — ㄷ자 골목을 따라 4~5점 | 첫 교전. 둘이 같은 경로를 다른 지점에서 돈다 |
| E 광장 | X +2500~+3500, Y −1500~+1500 | 1 | `PR_Plaza` — 광장 가장자리 3점 | 골목 출구에서 보이게 |
| F 1F | X +3500~+6500, Y −2000~+2000 | 1 | `PR_F1` — 실내 2~3점 | 창고 문 앞을 지나게 |
| F 2F | 같은 범위, 2층 | 1 | `PR_F2` — 창가 포함 2~3점 | 창문에서 광장이 보이는 지점 |

## 절차

### 1. 경로 놓기

1. Place Actors → 검색 `PatrolRoute` → 구역 바닥에 놓는다. 액터 위치가 곧 경로의 기준점
2. Details → Neon District → **Points** → `+` 로 점 추가 (3~5개)
3. 뷰포트에서 액터를 선택하면 점마다 **끌 수 있는 위젯**이 보인다. 순찰 동선대로 끌어 놓는다
4. **Loop** — 순환이면 체크, 왕복이면 해제
5. 액터 선택 → Data Layers Outliner → `DL_Mission` 우클릭 → **Add Selected Actors to Data Layer**
6. 이름을 `PR_Alley` 처럼 바꿔둔다

점은 **NavMesh 위**여야 한다. `P` 키를 눌러 초록 위에 있는지 확인. 벽 안이나 볼륨 밖이면 Move To가 즉시 실패하고 Idle만 반복한다.

### 2. 적 놓기

1. `Content/NeonDistrict/Blueprints/BP_NeonDistrictEnemy` 를 끌어다 놓는다. 발이 바닥에 닿게 (캡슐 절반 높이 만큼 띄움)
2. Details → 컴포넌트 목록 → **Mission Enemy** 클릭 → **Patrol Route** 드롭다운 → 그 구역의 경로 선택
3. 액터 선택 → `DL_Mission` 에 추가
4. 같은 경로를 도는 적은 **경로의 서로 다른 점 근처**에 놓는다. 첫 진입 때 가장 가까운 점부터 돌기 시작하므로 자연스럽게 흩어진다

### 3. 저장과 확인

- **Ctrl+S (Save All)**. PIE는 메모리 값으로 돌아서 저장 없이도 되지만 커밋엔 안 들어간다 (세 번 겪음)
- 플레이 → Fixer 수락 → 트리거 진입 → Output Log:
  - `[Warehouse] 적 등록 - N명` — N이 배치한 수와 같아야 한다
  - `[Patrol]` 경고가 **없어야** 한다. 있으면 아래 표
- 적이 점을 순서대로 걷고 점마다 2초쯤 멈춘다. 플레이어를 보면 추격·사격
- 다 잡으면 `남은 0명` → 키 드롭

## 자주 걸리는 것

| 증상 / 로그 | 원인 | 조치 |
|---|---|---|
| `[Patrol] 경로 없음(컴포넌트 있음)` | Patrol Route 미지정 또는 점 0개 | 컴포넌트에서 경로 선택, Points 확인 |
| `[Patrol] 경로 없음(컴포넌트 없음)` | 에디터 켠 채 빌드해서 컴포넌트 클래스가 재생성됨 | 에디터 끄고 번호 DLL 정리 후 빌드 |
| `[Patrol] Actor 컨텍스트가 비어 있음` | 트리 태스크의 Actor가 Context에 안 묶임 | ST_NeonDistrictEnemy → 태스크 → Actor 바인딩 |
| `[Patrol] Next Location이 바인딩되지 않음` | 파라미터 바인딩 누락 | 태스크 → Next Location → `Parameters → TargetMovement_Location` |
| 로그는 정상인데 2초마다 제자리 | 점이 NavMesh 밖 | `P` 키로 확인, 경로를 초록 위로 |
| `Could not find context actor of type BP_ShooterNPC_C` | 적 BP가 BP_ShooterNPC 자식이 아님 | 재부모가 아니라 **Create Child Blueprint Class** 로 만든다 |
| 적이 등록 수에 안 잡힘 | `DL_Mission` 밖이라 미션 시작 전에 BeginPlay가 돌았음 | 레이어에 추가 |
| 수락 전부터 적이 보임 | 위와 같음 | 레이어에 추가 |
| 죽어도 적이 안 돌아옴 | 위와 같음 | 레이어에 추가 |

## 관련

- `docs/level_flow.md` — 구역 좌표, 비트 시퀀스, 적 수
- `docs/neon_district_code.md` 11절 — 레이어 리셋이 적을 되돌리는 원리
- 커밋 `b47be2d` (적 배치·키 드롭), `5f19453` (순찰)
