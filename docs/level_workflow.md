# Neon District — 레벨 구성 및 원경 최적화 워크플로우

이 문서는 `Lvl_NeonDistrict` 맵을 만들고 원경(배경 도시)을 최적화하는 작업 순서와 규칙을 정의한다.
목표는 "예쁜 도시"가 아니라 **작은 플레이 공간이 거대한 도시처럼 보이게 만들고, 그 과정을 수치로 증명하는 것**이다.

- 엔진: Unreal Engine 5.8 (World Partition, Lumen, Virtual Shadow Maps, Substrate)
- 작업 맵: `/Game/NeonDistrict/Maps/Lvl_NeonDistrict`
- 플레이 구역: 150m × 150m (월드 원점 기준 ±7500cm)

---

## 1. 핵심 원칙 — 링(Ring) 레이어 구조

원경 최적화는 개별 기법의 나열이 아니라 **거리 대역마다 다른 규칙을 적용하는 구조**에서 시작한다.
모든 배치 판단은 "이 오브젝트는 어느 링에 속하는가"로 환원된다.

```
Ring 3   스카이 백드롭   (4km +)      : SkyAtmosphere / 안개 / 큐브맵. 지오메트리 없음
Ring 2   원경 스카이라인 (1.5 ~ 4km)  : 실루엣 전용. 단순 프록시, HLOD 단일 병합
Ring 1   중경 도시 블록  (300m ~ 1km) : 반복 키트 + ISM/HISM, HLOD Merged Mesh
Ring 0.5 경계 파사드     (~300m)      : 플레이 영역을 막는 벽. 정면만 존재, 내부 없음
Ring 0   플레이 구역     (150m)       : 콜리전 / 내비게이션 / 인터랙션이 존재하는 유일한 영역
```

### 링별 제작 규칙

| | Collision | Navigation | Material | Texture | Light Actor | HLOD |
|---|---|---|---|---|---|---|
| Ring 0 | O | O | Master MI, 디테일 O | 2K | 로컬 라이트 + 일부 그림자 | 불필요 |
| Ring 0.5 | 단순 박스 1개 | X | 동일 Master, 파라미터만 | 1K | Emissive만 | Instancing |
| Ring 1 | **없음** | X | 단순 Master (Emissive 창문) | 512 | **없음** | Merged Mesh |
| Ring 2 | **없음** | X | Unlit급 Emissive | 256 | **없음** | Simplified / Approximated |

**절대 규칙**

1. 원경 창문 불빛은 라이트 액터로 만들지 않는다. Emissive 머티리얼로만 표현한다.
   네온 씬에서 프레임을 죽이는 것은 폴리곤 수가 아니라 **그림자를 캐스팅하는 동적 라이트 개수**다.
2. Ring 1 이상에는 콜리전을 절대 넣지 않는다 (`Collision Presets = NoCollision`).
3. 새 에셋을 배치할 때 먼저 링을 정하고, 그 링의 규칙을 벗어나면 배치하지 않는다.
4. 원경은 "형상"이 아니라 **실루엣 + 안개 + 겹침**으로 거리감을 만든다. 디테일을 올리지 말고 레이어를 늘린다.

---

## 2. 폴더 및 네이밍 규칙

### 콘텐츠 폴더

```
/Game/NeonDistrict/
  Maps/            Lvl_NeonDistrict (World Partition)
  Blockout/        그레이박스 전용 메시 (아트 교체 시 폐기)
  Kit/
    Buildings/     모듈러 건물 조각
    Props/         소품
    Signs/         네온 간판
  DataLayers/      DL_* (Data Layer 에셋)
  Materials/       M_City_Master + MI_*
  Distant/
    Blocks/        Ring1/2 Level Instance (LI_CityBlock_*)
    Proxies/       원경 프록시 메시 / 베이크 텍스처
  HLOD/            HLODLayer 에셋
  Sequences/       컷씬 및 측정용 카메라 시퀀스
```

### 접두사

| 접두사 | 의미 | 예시 |
|---|---|---|
| `BLK_` | 블록아웃 지오메트리 | `BLK_Ground_PlayArea` |
| `REF_` | 스케일 기준 게이지 | `REF_Gauge_Cover_150` |
| `DIST_R1_` / `DIST_R2_` | Ring 1 / Ring 2 원경 오브젝트 | `DIST_R2_03` |
| `LI_` | Level Instance | `LI_CityBlock_A` |
| `CAM_Perf_` | 성능 측정용 고정 카메라 | `CAM_Perf_01_StreetEntrance` |
| `HLOD_` | HLODLayer 에셋 | `HLOD_Ring1_Merged` |

### 아웃라이너 폴더

```
Blockout/          Blockout/RingBoundary, Blockout/ScaleRef
Distant/           Distant/Ring1, Distant/Ring2
Lighting/
Gameplay/
Perf/              측정용 카메라
```

### 액터 태그

`Ring0`, `Ring05`, `Distant_Ring1`, `Distant_Ring2`, `ScaleRef`, `PerfCam` 태그를 배치 시점에 부여한다.
링 단위 일괄 설정(콜리전 해제, HLOD 레이어 지정, Cull Distance 조정)을 태그 기준으로 처리하기 위함이다.

---

## 3. 현재 맵 구성 상태

`Lvl_NeonDistrict`는 `Lvl_FirstPerson`(World Partition 맵)을 복제해 템플릿 아레나 지오메트리 55개를 제거한 상태에서 시작했다.

| 요소 | 내용 |
|---|---|
| `BLK_Ground_PlayArea` | 150m × 150m 지면. 윗면이 `Z = 0` |
| `BLK_Boundary_N/S/E/W` | ±7500cm 경계 파사드 벽 (높이 30m). Ring 0.5의 위치 기준선 |
| `REF_Gauge_*` | 엄폐 100cm / 150cm, 문 폭 200cm 스케일 기준물 |
| `DIST_R1_00 ~ 11` | Ring 1 스텁 블록 12개 (반경 450 ~ 670m, 높이 80 ~ 200m) |
| `DIST_R2_00 ~ 07` | Ring 2 스텁 블록 8개 (반경 1.5 ~ 2.6km, 높이 250 ~ 500m) |
| `CAM_Perf_01 ~ 05` | 고정 측정 시점 5개 |
| Lighting | DirectionalLight / SkyLight / SkyAtmosphere / ExponentialHeightFog / VolumetricCloud |
| `PlayerStart` | `(-6600, 0, 120)`, 거리 진입 방향 |

원경 스텁 블록은 최종 에셋이 아니라 **실루엣 스케일 기준이자 최적화 실험 대상**이다.
Phase 3에서 실제 키트로 교체하되, 블록 개수와 위치는 기준선 측정과의 비교를 위해 크게 바꾸지 않는다.

### 측정 카메라

| 카메라 | 위치 | 의도 |
|---|---|---|
| `CAM_Perf_01_StreetEntrance` | 거리 진입부 | 원경이 가장 넓게 열리는 시점 |
| `CAM_Perf_02_AlleyInterior` | 골목 내부 | 원경이 가려진 시점 (실내/협소 공간 기준선) |
| `CAM_Perf_03_Hideout2FWindow` | 적 건물 2층 | 실내에서 창밖 원경을 보는 최악 조합 |
| `CAM_Perf_04_CombatZone` | 전투 구역 중앙 | AI + 이펙트가 함께 도는 실사용 시점 |
| `CAM_Perf_05_SkylineOverlook` | 상공 40m | 원경 부하만 순수하게 보는 시점 |

---

## 4. 작업 순서

각 Phase 종료 시 반드시 측정하고 커밋한다. 순서를 지키는 것 자체가 포트폴리오의 내용이 된다.

### Phase 0 — 측정 기준선 (선행 필수)

최적화 전 수치가 없으면 "전후 비교"를 만들 수 없다. 아트를 시작하기 전에 측정 체계를 먼저 세운다.

1. `docs/perf/README.md`의 측정 프로토콜대로 환경을 고정한다.
2. 5개 고정 카메라에서 수치를 캡처한다.
3. `docs/perf/baseline.md`에 기록하고 커밋한다.

### Phase 1 — 그레이박스 (Ring 0 전용)

- `BLK_` 큐브만으로 150m × 150m 동선을 완성한다: 거리 → 상점 골목 → NPC 구역 → 전투 골목 → 적 건물 1F/2F.
- 확정할 메트릭: 문 폭 200cm, 엄폐물 높이 100/150cm, 통로 폭 최소 300cm, 층고 400cm.
- 원경이 열리는 시야 각도를 이 단계에서 정한다. **원경은 이 앵글에서만 잘 보이면 된다.**
- 기존 `BP_ShooterGameMode`를 물려 그레이박스 상태로 전투 테스트까지 마치고 동선을 잠근다.

### Phase 2 — 모듈러 키트 제작

- 건물 종류를 늘리지 않는다. 베이스 메시 6 ~ 8종 + 머티리얼 인스턴스 변형으로 해결한다.
- 트림시트 1 ~ 2장, 마스터 머티리얼 1개(파라미터: 베이스 컬러, 네온 색, 창문 밀도, 오염도).
- ComfyUI는 텍스처 / 네온 간판 아틀라스 / 콘셉트 보드 생성에 사용한다.

### Phase 3 — 원경 배치

- Ring 1/2 블록을 **Level Instance**(`LI_CityBlock_A/B/C`) 단위로 구성한다. 수정과 통계 관리가 쉬워진다.
- 반복 건물은 스플라인 또는 PCG로 배치한 뒤 ISM/HISM으로 병합한다.
- 안개 밀도를 링별로 미세하게 다르게 잡아 실루엣이 겹치도록 만든다.
- 배치 직후 측정한다. **이 시점이 최적화 전 수치**이며, 이후 모든 개선의 기준이 된다.

### Phase 4 — 최적화 패스 (한 번에 하나씩)

각 항목을 개별 커밋으로 분리하고 매번 측정한다. 무엇이 얼마나 기여했는지 말할 수 있어야 한다.

1. **World Partition 런타임 그리드**
   기본 셀 크기로는 150m 맵 전체가 셀 하나에 들어가 의미가 없다. 원경까지 포함해 맵을 2 ~ 4km로 깔고
   링별 Runtime Partition을 나눠 Loading Range를 다르게 준다. `wp.Runtime.ToggleDrawRuntimeHash2D`로 확인.
2. **HLOD 레이어 설계**
   Ring 1은 `Merged Mesh`(머티리얼 유지, 드로콜 병합), Ring 2는 `Simplified Mesh` 또는 `Approximated Mesh`.
   빌드 후 `wp.Runtime.HLOD 0/1`로 켜고 끄며 비교하면 그대로 전후 비교 자료가 된다.
3. **Nanite 판단 (핵심 실험)**
   동일한 원경을 두 버전으로 만들어 비교한다.
   - A안: Nanite 활성 원경
   - B안: 수동 LOD + HISM + Cull Distance
   `r.Nanite 0/1`과 두 버전의 수치를 함께 제시한다. Masked/Translucent(간판, 철망, 유리)는 Nanite에 불리하므로
   해당 요소를 분리해 측정한다.
4. **컬링**
   Cull Distance Volume, 컴포넌트별 Min/Max Draw Distance 설정. HLOD가 대체하는 액터와 컬 거리 설정이
   충돌하지 않도록 정리한다. `r.VisualizeOccludedPrimitives 1`로 오클루전 상태를 확인한다.
5. **라이트 / 그림자 예산**
   그림자 캐스팅 라이트 상한을 정한다(예: 씬 전체 8개). 나머지는 `Cast Shadows = off` + 작은 감쇠 반경.
   Virtual Shadow Map 비용을 별도로 확인한다.
6. **Lumen 품질 조정**
   젖은 노면 반사는 비싸다. Reflection Quality를 씬 기준으로 조정하고, 원경은 Lumen Scene에서 제외
   (`Affect Distance Field Lighting = off`)하는 것을 검토한다.

### Phase 5 — 라이팅 / 포스트 폴리싱 → 최종 재측정

- 네온 라이팅, PostProcess, 안개 폴리싱.
- 5개 카메라 전체 재측정 후 `docs/perf/`에 최종 비교표를 정리한다.

---

## 5. 에디터에서 수동으로 해야 하는 설정

아래 항목은 스크립트로 자동화되지 않으므로 에디터에서 직접 처리한다.

1. **HLODLayer 에셋** — 생성 및 액터 지정 완료
   `/Game/NeonDistrict/HLOD`에 두 레이어가 있고, 원경 액터 20개에 지정되어 있다.

   | 에셋 | Layer Type | 대상 |
   |---|---|---|
   | `HLOD_Ring1_Merged` | MeshMerge | `Distant_Ring1` 태그 12개 |
   | `HLOD_Ring2_Simplified` | MeshSimplify | `Distant_Ring2` 태그 8개 |

   `Cell Size 25600` / `Loading Range 76800`은 아직 기본값이다. Phase 4에서 링별 거리에 맞춰 조정하고
   그 전후를 측정한다. Ring 2는 1.5 ~ 2.6km에 있으므로 Loading Range를 크게 잡아야 한다.

   World Settings의 `Default HLOD Layer`는 None으로 둔다. 지정하면 블록아웃과 플레이 구역 액터까지
   HLOD 대상이 되므로, 링별로 명시 지정하는 현재 방식이 의도에 맞다.

2. **World Partition Runtime Partition 설정**
   World Settings → World Partition Setup에서 링별 그리드와 Loading Range를 구성한다.
   (RuntimeHash는 스크립트 API로 노출되지 않는다.)

3. **Data Layer 인스턴스 등록 및 액터 할당**
   에셋 4종은 `/Game/NeonDistrict/DataLayers`에 이미 생성되어 있다.

   | 에셋 | Type | 용도 |
   |---|---|---|
   | `DL_Blockout` | Runtime | 그레이박스 지오메트리. 아트 완성 후 Unloaded로 전환 |
   | `DL_Gameplay` | Runtime | PlayerStart, NPC, 스포너, 트리거 |
   | `DL_Art_Ring0` | Runtime | 플레이 구역 아트 |
   | `DL_Distant` | Runtime | Ring 1 / Ring 2 원경 |

   **플레이어가 밟고 서는 지오메트리는 Editor 타입 레이어에 넣지 않는다.** Editor 타입 데이터 레이어의 액터는
   쿡된 빌드에 포함되지 않으므로, 블록아웃 지면을 Editor 레이어에 넣으면 Standalone 측정에서 바닥이 사라진다.
   Editor 타입은 순수한 에디터 보조 액터(주석, 참조 마커)에만 쓴다.

   에셋만으로는 동작하지 않는다. 레벨마다 **DataLayerInstance를 등록**해야 한다.

   1. `Lvl_NeonDistrict`를 연다.
   2. `Window > World Partition > Data Layers`로 Data Layers 패널을 연다.
   3. Content Browser에서 에셋 4개를 선택해 Data Layers 패널로 드래그한다.
      (패널에서 우클릭 후 기존 Data Layer Asset을 추가하는 메뉴를 써도 된다.)
   4. 인스턴스의 `Initial Runtime State`를 정한다. **기본값이 Unloaded이므로 반드시 바꿔야 한다.**
      `DL_Blockout` / `DL_Gameplay` / `DL_Art_Ring0` / `DL_Distant` → 모두 Activated.
      Unloaded로 두면 PIE와 Standalone에서 해당 레이어의 액터가 아예 로드되지 않는다
      (블록아웃 지면이 사라져 플레이어가 낙하한다).
      원경을 껐다 켜며 측정할 때만 `DL_Distant`를 임시로 Unloaded로 바꾼다.
   5. 레벨에서 액터를 선택하고 Data Layers 패널의 레이어 행으로 드래그하거나,
      패널에서 우클릭 후 선택한 액터를 해당 레이어에 추가한다.

   액터 할당은 액터의 `DataLayerAssets` 프로퍼티에 저장되므로, 인스턴스만 등록해 두면
   태그(`Ring0` / `Ring05` / `Distant_Ring1` / `Distant_Ring2` / `ScaleRef`) 기준으로 스크립트 일괄 할당이 가능하다.

   목적은 두 가지다. 그레이박스와 아트를 병행 작업하는 것, 그리고 측정할 때 레이어 단위로 켜고 꺼서
   어떤 요소가 부하를 만드는지 분리하는 것이다.

4. **원경 액터 일괄 설정**
   `Distant_Ring1` / `Distant_Ring2` 태그로 선택 후 콜리전 해제, 그림자 캐스팅 해제, HLOD 레이어 지정.

5. **프로젝트 기본 맵 변경**
   현재 `Config/DefaultEngine.ini`의 `GameDefaultMap`은 `Lvl_FirstPerson`이다.
   작업 맵으로 전환할 준비가 되면 `Lvl_NeonDistrict`로 변경한다.

---

## 6. 측정

측정 프로토콜과 기록 양식은 다음 문서를 따른다.

- `docs/perf/README.md` — 측정 환경 고정 방법과 콘솔 명령
- `docs/perf/_template.md` — 측정 기록 템플릿
- `docs/perf/baseline.md` — 최초 기준선 기록

---

## 7. 스크립트 배치 시 주의

`/Game/LevelPrototyping/Meshes/SM_Cube`는 **피벗이 중심이 아니라 최소 코너**에 있다 (로컬 바운즈 0 ~ 100).
즉 액터 위치를 그대로 주면 블록이 `+X / +Y / +Z` 방향으로만 확장된다.
스크립트나 PCG로 블록을 배치할 때는 중심 좌표를 코너 좌표로 변환해야 한다.

```
location = (centre_x - width/2, centre_y - depth/2, z_base)
scale    = (width/100, depth/100, height/100)
```

배치 후에는 `get_actor_bounds`로 한 번 검증한다. 특히 원경 블록이 지면에서 떠 있으면
실루엣과 안개가 어긋나므로 `min.z == 0` 을 확인한다.
