# Neon District — 스토리

버티컬 슬라이스의 서사. 로드맵의 비트 시퀀스와 `DT_FixerDialogue`가 이 문서를 기준으로 한다.
이야기의 핵심은 간단하다. **갱단이 훔친 데이터 칩을 회수해 Fixer에게 돌려준다.**

## 전제

플레이어는 돈을 받고 위험한 일을 처리하는 용병이다.
어느 날 Fixer에게서 연락이 온다. 갱단이 그의 데이터 칩을 훔쳐 골목 끝 창고에 숨겨 두었다.
플레이어는 창고의 적을 처리하고 데이터 칩을 되찾아야 한다.

**Fixer** — 플레이어에게 일을 주고 보수를 지급하는 중개인.

**데이터 칩** — 갱단이 훔쳐 간 Fixer의 물건. 이번 미션의 회수 대상이다.

## 비트 시퀀스

| 단계 | 게임 | 이야기 |
|---|---|---|
| 거리 | 스폰 → 대로 탐색 | 플레이어가 Fixer를 찾아간다 |
| 의뢰 | Fixer E → `Intro_1~4` | Fixer가 도난당한 데이터 칩을 회수해 달라고 한다 |
| 진입 | 트리거 → `Fighting` | 플레이어가 갱단이 점거한 창고에 진입한다 |
| 전투 | 적 처치 | 창고를 지키는 갱단을 처리한다 |
| 키 | `KeyDropped → KeyAcquired` | 마지막 적이 떨어뜨린 창고 열쇠를 획득한다 |
| 창고 | 문 → 칩 → `ItemAcquired` | 잠긴 문을 열고 데이터 칩을 회수한다 |
| 복귀 | Fixer E → `Return_1~3` | 데이터 칩을 Fixer에게 전달하고 보수를 받는다 |
| 엔딩 | `Completed` | 미션 결과와 랭크가 표시된다 |

## 대사 — `DT_FixerDialogue`

| Row | 화자 | 본문 | Next | Effect |
|---|---|---|---|---|
| `Intro_1` | Fixer | 일 하나 있다. | `Intro_2` | |
| `Intro_2` | Fixer | 갱단이 내 데이터 칩을 훔쳐 갔어. 골목 끝 창고에 있을 거야. | `Intro_3` | |
| `Intro_3` | Fixer | 놈들을 처리하고 칩을 가져와. 보수는 확실히 주지. | `Intro_4` | |
| `Intro_4` | Player | 알았어. | | `AcceptMission` |
| `InProgress_1` | Fixer | 아직 칩을 못 찾았나? 창고 안쪽을 확인해 봐. | | |
| `Return_1` | Fixer | 칩은 가져왔나? | `Return_2` | |
| `Return_2` | Fixer | 확인했다. 약속한 보수는 넣어 뒀어. | `Return_3` | |
| `Return_3` | Player | 다음 일이 있으면 연락해. | | `CompleteMission` |
| `Completed_1` | Fixer | 오늘 일은 끝났어. | | |

시작 행 이름(`Intro_1`, `InProgress_1`, `Return_1`, `Completed_1`)과 Effect는 기존 NPC 프로퍼티 및 코드와 동일하게 유지한다.

## 목표 문구

| 상태 | HUD 목표 |
|---|---|
| `Accepted` | 창고로 이동하세요 |
| `Fighting` | 창고를 지키는 적을 처리하세요 |
| `KeyDropped` | 창고 열쇠를 획득하세요 |
| `KeyAcquired` | 잠긴 창고 문을 여세요 |
| `ItemAcquired` | Fixer에게 돌아가세요 |
| `Completed` | 미션 완료 |

## 엔딩

```text
Return_3 을 넘기는 순간
  → CompleteMission → OnStateChanged(Completed)
  → 화면 페이드 아웃
  → 미션 결과 표시:
        MISSION COMPLETE
        회수

        RANK   S
        DEATHS 0
  → 잠시 유지 후 종료
```

엔딩은 별도의 반전 없이 데이터 칩을 전달하고 보수를 받은 결과만 보여준다.

## 구현 자리 (로드맵 7·8번)

- `Completed` 방송 → `ANeonDistrictPlayerController`가 구독 → 입력 잠금 → 결과 화면 표시.
- 결과 위젯 `WBP_MissionResult`는 완료 시점에 `GetRank()`와 `GetDeathCount()`를 읽어 표시한다.
- `KeyDropped`는 창고를 지키는 마지막 적이 죽었을 때 전환한다.
- 데이터 칩은 기존 `AWarehousePickup`을 사용하고, 획득 시 `ItemAcquired`로 전환한다.
