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
