# [00] 기준선 — 원경 스텁 배치 상태

- 측정일: (미측정)
- 커밋:
- 변경 내용: 최적화 이전. 블록아웃 지면 + 경계 파사드 + Ring1/Ring2 스텁 블록 20개, HLOD 미빌드
- 맵 / 상태: `Lvl_NeonDistrict` / Phase 0

## 측정 대상 구성

| 요소 | 수량 |
|---|---|
| Ring 1 스텁 블록 | 12 |
| Ring 2 스텁 블록 | 8 |
| 경계 파사드 | 4 |
| 지면 | 1 |
| 그림자 캐스팅 라이트 | 1 (DirectionalLight) |

이 상태에는 HLOD, ISM, Cull Distance, Data Layer가 **적용되어 있지 않다.**
이후 모든 최적화 패스는 이 수치를 기준으로 비교한다.

## 환경

| 항목 | 값 |
|---|---|
| 실행 방식 | Standalone 1920×1080 |
| GPU / CPU | |
| ScreenPercentage / VSync / MaxFPS | 100 / 0 / 0 |

## 결과

| 카메라 | Frame (ms) | Game (ms) | Draw (ms) | GPU (ms) | FPS | DrawPrimitive calls |
|---|---|---|---|---|---|---|
| 01 StreetEntrance | | | | | | |
| 02 AlleyInterior | | | | | | |
| 03 Hideout2FWindow | | | | | | |
| 04 CombatZone | | | | | | |
| 05 SkylineOverlook | | | | | | |

## GPU 구간 (ProfileGPU — CAM_Perf_05 기준)

| 구간 | ms |
|---|---|
| Base Pass | |
| Shadow Depths / VSM | |
| Lumen | |
| Translucency | |

## 메모

- 스텁 블록은 단순 큐브이므로 이 시점의 GPU 부하는 낮게 나온다.
  실질적인 기준선은 Phase 3에서 실제 키트로 교체한 직후 다시 측정해 갱신한다.
- 그 갱신 측정은 `01_distant_kit_placed.md`로 별도 기록한다.
