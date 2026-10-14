# [02] 아트 패스 이후 기준선 재측정 — 나나이트 전환 비용과 VSM 경고 해소

- 측정일: 2026-10-14
- 커밋: `1c17da8` 시점의 맵
- 변경 내용: 건물·창문 스태틱 메쉬 12개에 **Nanite 활성화** (VSM 경고 해소 목적)
- 맵 / 상태: `Lvl_NeonDistrict` / 아트 패스 완료 상태 (건물·간판·네온·볼류메트릭 포그 배치 완료)

## 이 측정의 목적

두 가지를 동시에 확인한다.

1. **아트 패스 이후의 기준선.** `baseline.md`의 수치는 원경 스텁만 놓인 Phase 0 상태였다.
   건물과 네온 간판, 볼류메트릭 포그가 들어간 지금의 실제 비용을 다시 잡아 둔다.
2. **나나이트 활성화의 비용.** 매 프레임 로그를 채우던
   `Non-Nanite Marking Job Queue overflow` 경고를 없애려고 메쉬 12개에 나나이트를 켰다.
   경고가 사라지는 대신 프레임을 얼마나 내주는지가 쟁점이다.

## 환경

Standalone 1920×1080, RTX 3060 Ti / Ryzen 5 5600X, Lumen · VSM · Substrate 활성.
`CsvProfile Frames=300`으로 캡처하고 **앞 60프레임을 버린 뒤 나머지 241프레임의 평균**을 기록했다.
Before는 10/13 세션, After는 10/14 세션에서 각각 **연속으로** 측정했다.

## 나나이트를 켠 메쉬

| 경로 | 메쉬 |
|---|---|
| `Content/MyAssets/` | `SM_building1_1`, `SM_SM_building1_2`, `SM_SM_building3`, `SM_SM_building4` |
| `Content/MyAssets/` | `SM_buildingwindow`, `SM_SM_buildingwindow4`, `SM_SM_buildingwindow5` |
| `Content/MyAssets/` | `SM_NeonMahjongSign` |
| `Content/MyAssets/Building_Parts/` | `SM_SM_SM_Window_full`, `SM_SM_Window2`, `SM_SM_Window_middle`, `SM_Window_Side` |

## 결과

### Before (나나이트 끔)

| 카메라 | Frame (ms) | Game (ms) | Render (ms) | GPU (ms) | FPS |
|---|---:|---:|---:|---:|---:|
| 01 StreetEntrance | 14.10 | 3.00 | 14.10 | 13.21 | 70.9 |
| 02 AlleyInterior | 12.74 | 2.81 | 12.72 | 11.94 | 78.5 |
| 03 Hideout2FWindow | 13.81 | 3.00 | 13.81 | 12.83 | 72.4 |
| 04 CombatZone | 14.22 | 2.93 | 14.20 | 13.29 | 70.3 |
| 05 SkylineOverlook | 13.86 | 2.82 | 13.84 | 13.06 | 72.2 |

### After (나나이트 켬)

| 카메라 | Frame (ms) | Game (ms) | Render (ms) | GPU (ms) | FPS |
|---|---:|---:|---:|---:|---:|
| 01 StreetEntrance | 14.35 | 2.89 | 14.34 | 13.37 | 69.7 |
| 02 AlleyInterior | 12.88 | 2.82 | 12.86 | 11.91 | 77.6 |
| 03 Hideout2FWindow | 13.24 | 2.69 | 13.23 | 12.29 | 75.5 |
| 04 CombatZone | 14.15 | 2.81 | 14.14 | 13.16 | 70.7 |
| 05 SkylineOverlook | 13.84 | 2.78 | 13.82 | 12.90 | 72.3 |

### 변화량

| 카메라 | Frame | GPU |
|---|---:|---:|
| 01 StreetEntrance | **+0.25** | +0.16 |
| 02 AlleyInterior | **+0.14** | −0.03 |
| 03 Hideout2FWindow | **−0.57** | −0.54 |
| 04 CombatZone | **−0.07** | −0.13 |
| 05 SkylineOverlook | **−0.02** | −0.16 |

### VSM 경고

| | `Non-Nanite Marking Job Queue overflow` 발생 |
|---|---|
| Before | 매 프레임 발생 |
| After | **22개 로그 전부 0건** |

## 해석

**나나이트 활성화 비용은 측정 한계 아래다.** 부호가 양쪽으로 섞여 있고 폭이 ±0.6ms인데,
`README.md`에 기록된 세션 간 편차가 3~4%(약 ±0.5ms)다. 즉 이 표의 차이는 전부 그 편차 안에 들어간다.
개선도 악화도 측정되지 않았고, **경고만 사라졌다.**

이 결과는 `01_nanite_overhead.md`와도 어긋나지 않는다. 그 문서에서 측정한 +1.0ms는
`r.Nanite 0 → 1`로 **나나이트 구성 자체를 켜는** 비용이었다. 구성은 이미 켜져 있었으므로
이번에 추가된 것은 메쉬 12개를 나나이트 경로에 올리는 비용뿐이고, 그건 공짜였다.

### 왜 콘텐츠 최적화를 더 진행하지 않는가

다섯 카메라 전부 12.7~14.4ms 안에 모여 있다. 시야에 건물이 가득한 01·04와
거의 비어 있는 02 사이의 차이가 1.5ms뿐이다. **이 씬의 비용은 콘텐츠가 아니라
Lumen · VSM · Substrate의 고정 비용이 지배한다.**

따라서 프랍을 줄이거나 LOD를 손보는 작업은 노이즈 수준의 변화만 만든다.
실제로 건물 콜리전을 convex hull 528개에서 13개로 줄였을 때도 프레임 변화는 측정되지 않았다
(그 작업의 가치는 프레임이 아니라 물리 안정성과 에디터 작업성이었다).
남은 일정은 최적화가 아니라 **버그 수정과 완주 품질**에 쓰는 것이 맞다고 판단했다.

70 FPS는 이 수직 슬라이스의 목표(60 FPS)를 충족한다.

## 측정 함정 — 이번에 측정을 세 번 버렸다

### 1. `taskkill //F //IM`이 Git Bash에서 조용히 실패한다

측정 스크립트는 카메라마다 에디터를 띄우고 캡처가 끝나면 종료시킨다.
그 종료 명령이 **한 번도 실행되지 않았다.**

Git Bash의 경로 변환(`/Game/...` → `C:/Program Files/Git/Game/...`)을 막기 위해
`MSYS2_ARG_CONV_EXCL="*"`를 걸어 두었는데, 이 설정은 **`//F`를 `/F`로 바꿔 주는 변환까지 끈다.**
그래서 taskkill은 `//F`라는 존재하지 않는 인자를 받고 실패했고, 실패를 조용히 삼켰다.

결과적으로 측정이 끝난 에디터가 전부 살아남아 **UnrealEditor.exe 7개가 각 4GB씩 동시에 실행**됐다.
새 측정은 매번 이전 에디터 여섯 개와 GPU를 나눠 쓰고 있었다.

버려진 수치들:

| 카메라 | 오염된 값 | 실제 값 |
|---|---:|---:|
| 02 | 21.12 ms | 12.88 ms |
| 03 | 16.60 → 24.53 ms | 13.24 ms |
| 04 | 18.12 → 24.44 ms | 14.15 ms |
| 05 | 102.02 ms | 13.84 ms |

**회차를 거듭할수록 수치가 올라가는 것**이 이 오염의 지문이다
(03이 16.6 → 24.5로 악화). 같은 조건의 재측정이 더 나빠지면 변경을 의심하기 전에
머신 상태를 의심해야 한다.

대책: Git Bash에서 프로세스를 죽일 때는 taskkill을 쓰지 않고 PowerShell을 쓴다.

```bash
powershell -NoProfile -Command "Get-Process UnrealEditor -ErrorAction SilentlyContinue | Stop-Process -Force"
```

그리고 **죽였는지 확인한 뒤에 측정을 시작한다.** 살아 있으면 측정하지 않고 중단한다.

```bash
n=$(powershell -NoProfile -Command "(Get-Process UnrealEditor -ErrorAction SilentlyContinue|Measure-Object).Count")
[ "${n//[^0-9]/}" = "0" ] || { echo "ABORT: $n editor(s) alive"; exit 2; }
```

Stop-Process도 한 번에 전부 죽이지 못할 때가 있다(7개 중 4개가 남았다). 몇 번 반복해야 한다.

### 2. 로그에 찍힌 모듈 로드를 오염 증거로 오인했다

처음 02가 21ms로 나왔을 때, `after_02.log`에 `LogModuleManager: InternalLoadLibrary: 'NaniteBuilder'`가
찍혀 있는 것을 보고 "나나이트 데이터를 처음 굽는 중에 쟀다"고 진단했다. **틀렸다.**

`before_*.log` 12개에도 `NaniteBuilder` 로드가 **똑같이 한 번씩** 찍혀 있다. 평범한 모듈 로드다.

교훈: 로그의 어떤 줄을 이상 징후로 지목하기 전에, **정상 로그에도 그 줄이 있는지 먼저 센다.**
대조군이 있는데도 쓰지 않았다.

### 3. 카메라 좌표를 기억에 의존했다

재측정 중 카메라 01을 `BugItGo -1200 1500 200 0 0 0`으로 쟀는데, 실제 01은
`BugItGo -6800 0 180 -3 0 0`이다. 전혀 다른 지점을 Before와 비교할 뻔했다.

좌표는 `README.md`의 표에 있고, 지난 측정의 로그에서도 복구할 수 있다.

```bash
grep -ao "BugItGo [-0-9. ]*" Saved/PerfLogs/before_0*.log | sort -u
```

### 4. CSV는 다 쓰이기를 기다려야 한다

`Saved/Profiling/CSV/`의 파일을 캡처 종료 직전에 복사하면 0바이트나 잘린 파일을 얻는다.
스크립트는 **실행 시작 시각(`T0=$(date +%s)`)보다 새로운 파일 중, 4초 간격으로 두 번 잰 크기가
같은 것**만 채택한다. 또 캡처 중인 프로세스가 파일을 잡고 있으면 복사가
`Device or resource busy`로 실패하므로, 에디터를 먼저 죽인 뒤 복사한다.

## 다음 액션

- 이 문서의 After 수치를 아트 패스 이후의 새 기준선으로 쓴다. `baseline.md`의 Phase 0 수치와는 비교하지 않는다.
- 남은 일정은 최적화 대신 버그 수정에 쓴다.
- `Content/MyAssets/`가 gitignore 대상이라 이번에 나나이트를 켠 메쉬 12개가 저장소에 남지 않는다. 이 자산들의 추적 여부를 결정해야 한다.
