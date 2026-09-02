# [NN] 측정 제목

- 측정일:
- 커밋:
- 변경 내용: (이번 패스에서 바꾼 것 **하나**를 한 줄로)
- 맵 / 상태: `Lvl_NeonDistrict` /

## 환경

| 항목 | 값 |
|---|---|
| 실행 방식 | Standalone 1920×1080 |
| GPU / CPU | |
| ScreenPercentage / VSync / MaxFPS | 100 / 0 / 0 |

## 결과

### Before

| 카메라 | Frame (ms) | Game (ms) | Draw (ms) | GPU (ms) | FPS | DrawPrimitive calls |
|---|---|---|---|---|---|---|
| 01 StreetEntrance | | | | | | |
| 02 AlleyInterior | | | | | | |
| 03 Hideout2FWindow | | | | | | |
| 04 CombatZone | | | | | | |
| 05 SkylineOverlook | | | | | | |

### After

| 카메라 | Frame (ms) | Game (ms) | Draw (ms) | GPU (ms) | FPS | DrawPrimitive calls |
|---|---|---|---|---|---|---|
| 01 StreetEntrance | | | | | | |
| 02 AlleyInterior | | | | | | |
| 03 Hideout2FWindow | | | | | | |
| 04 CombatZone | | | | | | |
| 05 SkylineOverlook | | | | | | |

### 변화량

| 카메라 | GPU 변화 | DrawPrimitive 변화 |
|---|---|---|
| 01 StreetEntrance | | |
| 05 SkylineOverlook | | |

## GPU 구간 (ProfileGPU, 가장 부하가 큰 카메라 기준)

| 구간 | Before (ms) | After (ms) |
|---|---|---|
| Base Pass | | |
| Shadow Depths / VSM | | |
| Lumen | | |
| Translucency | | |

## 해석

-

## 부작용 / 트레이드오프

- (팝핑, 실루엣 손실, 메모리 증가, 빌드 시간 등)

## 다음 액션

-
