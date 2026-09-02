# [00] 기준선 — 원경 스텁 배치 상태

- 측정일: 2026-09-02
- 커밋: `b44fda8` 시점의 맵
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

이 상태에는 HLOD 빌드, ISM, Cull Distance가 **적용되어 있지 않다.**
Data Layer는 4종 모두 Activated 상태다.

## 환경

| 항목 | 값 |
|---|---|
| 실행 방식 | Standalone (`-game`), 1920×1080 windowed |
| 엔진 | Unreal Engine 5.8.2 (Development) |
| GPU | NVIDIA GeForce RTX 3060 Ti (8GB) |
| CPU | AMD Ryzen 5 5600X |
| OS | Windows 11 (25H2) |
| ScreenPercentage / VSync / MaxFPS | 100 / 0 / 0 |
| 렌더링 | Lumen, Virtual Shadow Maps, Substrate, Ray Tracing 활성 |

## 측정 방법

CSV 프로파일러로 카메라당 5000프레임을 캡처하고, **마지막 2500프레임**(로딩과 초기 스파이크를 제외한
안정 구간, 유효 프레임 2499개)의 평균을 기록했다. 각 실행은 `ghost` + `BugItGo`로 고정 시점에 배치했다.

```
-ExecCmds="r.VSync 0,t.MaxFPS 0,r.ScreenPercentage 100,ghost,BugItGo <좌표>" -csvCaptureFrames=5000 -csvGpuStats
```

CSV의 메타데이터 행은 `FrameTime <= 0` 조건으로 걸러냈다. 거르지 않으면 타임스탬프 값이
카운터 컬럼에 섞여 드로우 콜 평균이 수십만으로 나온다.

## 결과

| 카메라 | Frame (ms) | Median (ms) | Game (ms) | Render (ms) | GPU (ms) | RHI (ms) | FPS | Draw Calls |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 01 StreetEntrance | 12.42 | 12.37 | 2.13 | 12.40 | 11.43 | 2.92 | 80.5 | 23 |
| 02 AlleyInterior | 12.40 | 12.37 | 2.09 | 12.39 | 11.42 | 2.85 | 80.6 | 23 |
| 03 Hideout2FWindow | 12.84 | 12.79 | 2.12 | 12.82 | 11.84 | 2.94 | 77.9 | 23 |
| 04 CombatZone | 12.68 | 12.63 | 2.10 | 12.66 | 11.68 | 2.93 | 78.9 | 23 |
| 05 SkylineOverlook | 13.42 | 13.40 | 2.11 | 13.41 | 12.39 | 2.87 | 74.5 | 23 |

### 드로우 콜 구성 (CAM_Perf_05 기준, 패스별 평균)

| 패스 | 드로우 콜 |
|---|---:|
| ShadowDepths | 13.6 |
| Lights | 2.9 |
| SlateUI | 1.9 |
| RenderVelocities | 1.9 |
| BasePass | 1.9 |
| Fog | 1.0 |
| **합계** | **23.2** |

## 해석

**GPU 바운드다.** RenderThread 시간이 FrameTime과 거의 같고(12.40 vs 12.42) GameThread는 2.1ms에 불과하다.
프레임 시간의 대부분이 GPU 대기다. 즉 이 프로젝트의 최적화는 게임플레이 코드가 아니라
렌더링 쪽에서 승부가 난다.

**카메라 간 차이가 1ms뿐이다.** 원경이 전혀 보이지 않는 골목(02, 12.40ms)과 도시 전체가 보이는
상공 오버룩(05, 13.42ms)의 차이가 1.02ms다. 지금 원경 스텁 20개가 만드는 부하는 프레임의 8%도 안 되고,
나머지는 Lumen / VSM / SkyAtmosphere / 볼류메트릭 안개 같은 **화면 전체에 걸리는 고정 비용**이다.

**드로우 콜은 이 구성에서 지표가 되지 못한다.** 어느 카메라에서도 23으로 동일하고 BasePass는 1.9뿐이다.
템플릿의 `SM_Cube`가 Nanite 메시라 지오메트리가 전통적인 드로우 콜을 쓰지 않기 때문이다.
Nanite 경로를 유지하는 한 "드로우 콜 xxx → yyy" 식의 비교는 성립하지 않으며,
**GPU 시간이 유일하게 의미 있는 지표**다. 드로우 콜을 비교 축으로 쓰려면
Phase 4의 B안(수동 LOD + HISM + Cull Distance, Nanite 비활성)을 함께 만들어야 한다.

## 이 수치의 한계

스텁 블록은 머티리얼 한 장짜리 단순 큐브다. 실제 키트로 교체하면 GPU 시간이 크게 올라간다.
따라서 **최적화 효과를 증명하는 실질적인 기준선은 Phase 3 직후에 다시 잡아야 한다.**
그 측정은 `01_distant_kit_placed.md`로 기록한다.

지금 이 표의 역할은 두 가지다. 첫째, 측정 파이프라인이 재현 가능하게 동작한다는 확인.
둘째, 아무것도 없는 상태의 고정 비용(약 11.4ms GPU)을 알아두는 것. 이후 어떤 수치를 보든
여기서 11.4ms를 빼면 그것이 실제로 추가된 콘텐츠의 비용이다.

## 다음 액션

- Phase 1 그레이박스 진행 후 재측정 (동선 확정 시점)
- Phase 3에서 실제 원경 키트 배치 직후 재측정 → 이것이 최적화 비교의 실질 기준선
- HLOD 빌드 후 `wp.Runtime.HLOD 0/1` 비교
