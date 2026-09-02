# 성능 측정 프로토콜

최적화 전후 비교가 의미를 가지려면 **측정 조건이 항상 동일해야 한다.**
이 문서의 절차를 벗어난 수치는 기록하지 않는다.

## 1. 측정 환경 고정

| 항목 | 값 | 비고 |
|---|---|---|
| 실행 방식 | Standalone Game | 에디터 뷰포트 수치는 에디터 오버헤드가 섞이므로 기록하지 않는다 |
| 해상도 | 1920 × 1080 고정 | |
| Screen Percentage | `r.ScreenPercentage 100` | TSR 스케일링이 수치를 왜곡하는 것을 막는다 |
| VSync | `r.VSync 0` | |
| 프레임 상한 | `t.MaxFPS 0` | 상한이 걸리면 개선폭이 보이지 않는다 |
| 하드웨어 | 매번 동일 PC / 동일 전원 프로파일 | 노트북은 전원 연결 상태 고정 |

**측정 전 필수 확인**

- 셰이더 컴파일이 완전히 끝났는가 (에디터 우하단 컴파일 카운터 0).
- 텍스처 스트리밍이 안정화되었는가 (해당 시점에서 5초 이상 대기 후 측정).
- 백그라운드에서 다른 빌드나 임포트가 돌고 있지 않은가.

첫 진입 프레임은 버리고, 각 카메라에서 **5초 대기 후 10초간의 안정 구간**을 기록한다.

## 2. 측정 항목과 명령

| 지표 | 명령 | 기록할 값 |
|---|---|---|
| 프레임 타임 | `stat unit` | Frame / Game / Draw / GPU (ms) |
| 드로우 콜 | `stat rhi` | DrawPrimitive calls |
| 씬 렌더링 | `stat scenerendering` | Mesh draw calls, Visible static mesh elements |
| GPU 구간별 비용 | `stat gpu` 또는 `ProfileGPU` (Ctrl+Shift+,) | Base Pass / Shadow Depths / Lumen / VSM |
| 텍스처 스트리밍 | `stat streaming` | Streaming Pool 사용량 |

### 원인 분리용 토글

| 목적 | 명령 |
|---|---|
| HLOD 기여도 확인 | `wp.Runtime.HLOD 0` / `1` |
| Nanite 기여도 확인 | `r.Nanite 0` / `1` |
| 스트리밍 그리드 시각화 | `wp.Runtime.ToggleDrawRuntimeHash2D` |
| 오클루전 확인 | `r.VisualizeOccludedPrimitives 1` |
| 컬링 상태 눈으로 확인 | `freezerendering` (카메라를 움직여 무엇이 컬링되었는지 관찰) |

장시간 구간을 파일로 남기려면 `CsvProfile Start` / `CsvProfile Stop`, 상세 분석이 필요하면 Unreal Insights를 사용한다.

## 3. 기록 방법

1. `_template.md`를 복사해 `NN_주제.md`로 만든다 (예: `01_hlod_ring1.md`).
2. **한 번에 하나의 변경만** 측정한다. 두 가지를 동시에 바꾸면 기여도를 분리할 수 없다.
3. 측정이 끝나면 해당 변경과 기록 파일을 같은 커밋에 포함한다.

## 4. 파일 목록

| 파일 | 내용 |
|---|---|
| `_template.md` | 기록 템플릿 |
| `baseline.md` | 최초 기준선 (원경 스텁 배치 상태) |

## 5. 자동 측정 실행 방법

에디터 UI를 거치지 않고 커맨드라인으로 Standalone을 띄워 고정 시점의 수치를 잡는다.
카메라마다 한 번씩 실행한다.

```
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" ^
  "C:\Projects\CyberPunkProject\CyberPunkProject.uproject" ^
  /Game/NeonDistrict/Maps/Lvl_NeonDistrict ^
  -game -windowed -resx=1920 -resy=1080 ^
  -ExecCmds="r.VSync 0,t.MaxFPS 0,r.ScreenPercentage 100,ghost,BugItGo -6800 0 180 -3 0 0,stat unit" ^
  -abslog="<로그 경로>"
```

- `-ExecCmds`의 값 전체를 **큰따옴표로 감싸야 한다.** 감싸지 않으면 공백에서 잘려
  `r.VSync`만 인식되고 나머지 명령이 전부 무시된다.
- `ghost`를 먼저 실행해 콜리전을 끈다. 이걸 빼면 공중 시점(`CAM_Perf_05`)에서 플레이어가 낙하해
  측정 위치가 유지되지 않는다.
- `BugItGo X Y Z Pitch Yaw Roll`로 이동한다. 로그에 `LogCheatManager: BugItGo to: ...`가 찍히면 성공이다.
- `stat unit`은 UE 5.8에서 `Draws`와 `Prims`까지 함께 표시하므로 드로우 콜 확인에 `stat rhi`를 따로 켤 필요가 없다.
- Data Layer의 `Initial Runtime State`를 바꿨다면 **레벨을 저장한 뒤** 실행한다.
  Standalone은 디스크의 상태를 읽으므로, 저장하지 않으면 레이어가 Unloaded인 채로 실행되어
  지면이 사라지고 플레이어가 낙하한다.

### 카메라별 BugItGo 인자

| 카메라 | 인자 |
|---|---|
| `CAM_Perf_01_StreetEntrance` | `BugItGo -6800 0 180 -3 0 0` |
| `CAM_Perf_02_AlleyInterior` | `BugItGo -2000 -3000 180 0 90 0` |
| `CAM_Perf_03_Hideout2FWindow` | `BugItGo 4500 0 700 -5 180 0` |
| `CAM_Perf_04_CombatZone` | `BugItGo 2000 0 180 0 -135 0` |
| `CAM_Perf_05_SkylineOverlook` | `BugItGo 0 0 4000 -8 45 0` |

### 측정 환경에 대한 경고

이 방식으로 얻은 수치는 **측정하는 동안 PC에서 다른 작업이 돌지 않을 때만** 유효하다.
언리얼 에디터를 켜둔 채로 측정하면 에디터가 GPU와 CPU를 함께 점유해 프레임 타임이
몇 배로 부풀려진다. 실제로 에디터를 켜둔 상태에서 잰 첫 시도는 거의 빈 블록아웃 씬에서
Frame 66ms(15 FPS)가 나왔다. 측정 전에 에디터와 브라우저, 메신저를 모두 종료하고,
측정 중에는 창을 전환하거나 마우스를 움직이지 않는다.
