# Week 1 — 풀숲 메카닉 단계별 가이드

> 목표: 캐릭터가 풀밭을 걸으면 풀이 캐릭터로부터 밀려나는 시각 효과. 끝나면 30초 이상 끊김 없이 풀숲 안에서 보행 가능.
>
> 예상 소요: 4~6시간 (입문자 기준, 잔디 다운로드 시간 제외)

---

## 0. 준비물 — 잔디 메쉬

### 옵션 A: Quixel Bridge (UE5 내장, 권장)
1. UE5 에디터 상단 `Window → Quixel Bridge` 열기
2. Epic 계정 로그인
3. 좌측 카테고리 `3D Plants → Grass`
4. **권장 선택:** "Wild Grass", "Field Grass", "Meadow Grass" 같은 키워드. **Nanite 호환** 표기 있는 것 우선.
5. 메쉬 1~2종 선택 → 우측 `Quality = High` 또는 `Medium` → `Download` → `Add`
6. Content Browser `Megascans/3D_Plants/...` 아래에 import됨

### 옵션 B: Fab (스타일라이즈드)
- "Stylized Grass Pack" 검색 → 무료 또는 $5~15
- Content Browser로 import

> 이번 가이드는 **Quixel 잔디 기준** 노드 그래프. 스타일라이즈드라도 거의 동일하나 VertexColor 사용처가 다를 수 있음 (3-1 단계 참조).

---

---

## 0-1. 첨부 에셋 — Ribbon Grass 수동 Import (이 프로젝트 기준)

다운로드한 폴더: `C:\Users\Thvmx.DEPTHS\Downloads\Plants_3d_tbdpec3r_4K_3dplant_ms`

**에셋 정보:**
- Quixel Megascans "Ribbon Grass" (Phalaroides arundinacea, 갈대 카나리풀)
- 6 Variant × 4 LOD FBX, 4K Atlas + Billboard 텍스처
- 트라이앵글 LOD0: 2200~3700, LOD3: 2 (빌보드)

### 0-1-1. 폴더 구조 준비
1. UE5 에디터 Content Browser에서 다음 폴더 생성:
   - `Content/Megascans/Plants/RibbonGrass/Meshes/`
   - `Content/Megascans/Plants/RibbonGrass/Textures/`

### 0-1-2. 텍스처 import (먼저)
1. Content Browser `RibbonGrass/Textures/` 안에서 우클릭 → `Import to /Game/Megascans/Plants/RibbonGrass/Textures/`
2. `...\Textures\Atlas\` 폴더의 7개 jpg 모두 선택 (Displacement는 .exr 또는 .jpg 둘 중 jpg만 일단 선택 — exr은 용량 큼)
3. **import 후 각 텍스처 우클릭 → Asset Actions → Bulk Edit via Property Matrix** 또는 개별 더블클릭하여 다음 설정:

| 텍스처 | sRGB | Compression Settings |
|---|---|---|
| `smencbp_4K_Albedo` | ✅ ON | `Default (DXT1/5)` |
| `smencbp_4K_Normal` | ❌ OFF | `Normalmap (DXT5, BC5 on PC)` |
| `smencbp_4K_AO` | ❌ OFF | `Masks (no sRGB)` |
| `smencbp_4K_Roughness` | ❌ OFF | `Masks (no sRGB)` |
| `smencbp_4K_Opacity` | ❌ OFF | `Masks (no sRGB)` |
| `smencbp_4K_Translucency` | ❌ OFF | `Masks (no sRGB)` |
| `smencbp_4K_Displacement` | ❌ OFF | `Masks (no sRGB)` (사용 안 하면 import 생략 가능) |

> sRGB 잘못 설정하면 색·빛 계산 어긋남. **Normal과 데이터 맵은 항상 sRGB OFF**.

4. Billboard 폴더 텍스처는 **LOD3 빌보드를 쓸 때만** import. Nanite Foliage 사용하면 빌보드 LOD 자동 무시 가능 → **이번 데모는 일단 생략 권장**.

### 0-1-3. 메쉬 import
**전략 결정:**
- **Nanite ON 사용 (UE5.3+ 권장):** LOD0만 import. Nanite가 자동으로 거리별 디테일 처리.
- **Nanite OFF (호환성·구버전):** LOD0~LOD2 import, FBX import 시 Static Mesh에 LOD 통합.

**이번 데모는 Nanite ON 권장.** 단순함 우선.

#### Nanite ON 경로 — LOD0만 import (권장)
1. Content Browser `RibbonGrass/Meshes/` 안에서 우클릭 → `Import to /Game/...Meshes/`
2. 파일 선택 — 6개 LOD0만:
   - `Var1/Var1_LOD0.fbx`
   - `Var2/Var2_LOD0.fbx`
   - ... `Var6/Var6_LOD0.fbx`
3. FBX Import Options 다이얼로그:
   - `Import Mesh` ✅, `Static Mesh`
   - `Combine Meshes` ❌ OFF (각자 별개 SM이 되도록)
   - `Auto Generate Collision` ❌ OFF (Foliage는 콜리전 끔)
   - `Generate Lightmap UVs` ✅ ON
   - `Import Materials` ❌ OFF (직접 만들 거라)
   - `Import Textures` ❌ OFF (이미 따로 import했으므로)
   - `Import` 클릭
4. Import 후 6개 SM 생성됨 (`SM_Var1_LOD0` 등)
5. **Bulk Edit via Property Matrix** 또는 개별로 다음 설정:
   - `Nanite Settings → Enable Nanite Support` ✅ ON
   - `LODGroup` = `Foliage`
   - `Collision Complexity` = (콜리전 안 쓰니 무관)
   - `Two Sided` 관련 설정은 머티리얼에서 처리

> **Nanite WPO 주의 (UE5.3+):** Nanite 메쉬에 World Position Offset 머티리얼 적용 시 메쉬 Detail의 `Nanite → World Position Offset Disable Distance` 값 확인 (기본 50000 정도). 가까이서만 휘면 충분하므로 기본값 유지.

#### Nanite OFF 경로 (대안)
Import Options에서 `LOD Group = Foliage`. 각 변형마다 LOD0~LOD3 4개를 한 SM의 LOD로 통합하려면 SM 에디터에서 LOD Settings → `Number of LODs = 4` → 각 LOD에 수동 import. 입문자는 복잡 → Nanite ON 권장.

### 0-1-4. 머티리얼 설정 보정 (★ 이 에셋 특화)

가이드 2번 섹션의 `M_Grass_Bend` 작성 시 **이 에셋용 변경점**:

#### Shading Model을 `Two Sided Foliage`로
2-1 단계에서:
- `Shading Model` = **`Two Sided Foliage`** (Default Lit 아님)
- `Two Sided` ✅ (자동으로 켜질 수도)
- `Blend Mode` = `Masked`
- `Opacity Mask Clip Value` = `0.33`

`Two Sided Foliage` 모델은 Subsurface Color를 통해 잎의 빛 투과(translucency)를 표현. Journey풍 황혼 라이팅에서 잎이 빛에 비치는 효과의 핵심.

#### 텍스처 연결 (이 에셋 기준)

| 노드 | 텍스처 | Main 핀 |
|---|---|---|
| Texture Sample | `smencbp_4K_Albedo` | RGB → `Base Color` |
| Texture Sample | `smencbp_4K_Normal` | RGB → `Normal` |
| Texture Sample | `smencbp_4K_Roughness` | R → `Roughness` |
| Texture Sample | `smencbp_4K_AO` | R → `Ambient Occlusion` |
| Texture Sample | `smencbp_4K_Opacity` | R → `Opacity Mask` |
| Texture Sample | `smencbp_4K_Translucency` | R → **`Subsurface Color`** (Multiply로 색 입히기 권장) |

**Subsurface Color 권장 그래프:**
```
Translucency.R → Multiply(A) ─┐
Vector3(0.5, 0.8, 0.3) ─────→ B ─→ Subsurface Color
```
(녹색 톤으로 빛 투과 색 입힘. 색은 취향대로)

> **Albedo가 너무 어두우면:** Multiply로 1.2~1.5 곱해 밝게. 또는 Post Process Volume의 Exposure 조정.

#### VertexColor 마스크 → 다른 방법으로
**중요:** Quixel Megascans에서 직접 다운로드한 FBX는 보통 VertexColor가 **들어 있음**. 하지만 일부 변형은 비어 있을 수 있음. 검증 후 처리:

1. SM_Var1_LOD0 더블클릭 → Static Mesh Editor
2. 좌상단 `Show → Vertex Colors` 켜기
3. 메쉬가 검정~흰색 그라데이션이면 ✅ VertexColor 있음 → 가이드 2-5 E단계 그대로
4. 단색이거나 흰색이면 ❌ VertexColor 없음 → 다음 대체 마스크 사용:

**대체 마스크 (LocalPosition Z 기반):**
```
LocalPosition → ComponentMask(B) → Divide(B=30) → Saturate
```
이걸 가이드 2-5 E의 VertexColor.R 자리에 사용. `30`은 잔디 높이 (cm), 메쉬마다 조정.

### 0-1-5. 머티리얼 인스턴스 생성 → 메쉬에 적용
1. `M_Grass_Bend` 우클릭 → `Create Material Instance` → `MI_RibbonGrass`
2. MI 더블클릭 → 파라미터 켜고 조정:
   - `BendRadius` = 200
   - `BendStrength` = 80
   - `WindIntensity` = 0.3
   - `WindWeight` = 1.0
   - `WindSpeed` = 1.5
3. `SM_Var1_LOD0`~`SM_Var6_LOD0` 각각 열어서 Material Slot에 `MI_RibbonGrass` 적용
   - 또는 Bulk Edit via Property Matrix로 6개 한꺼번에

### 0-1-6. Foliage Type 구성 (랜덤 변형 활용)
6개 변형을 자연스럽게 섞으려면:

**옵션 A — Foliage Type 6개 (단순):**
- 각 메쉬마다 Foliage Type 생성 (`FT_RibbonGrass_Var1`~`Var6`)
- Foliage Mode에서 6개 모두 체크하고 함께 페인팅 → 자동 랜덤 분포

**옵션 B — Foliage Type 1개에 여러 메쉬 (UE5 5.0+ 지원 안 함, 기존 방식 X)**
- UE5 Foliage Type 1개당 메쉬 1개 정책. 옵션 A 사용.

**권장: 옵션 A.** 가이드 4번 섹션의 Foliage Type 만들기를 6번 반복. 또는 일단 Var1·Var3·Var5 3종만 만들어 시작 → 만족하면 나머지 추가.

각 Foliage Type 설정값 (가이드 4-1과 동일):
- Density: 2000
- Random Yaw ✅
- Align to Normal ✅
- Random Pitch Angle: 5
- Z Offset: -2 ~ 2
- Cast Shadow ❌
- Cull Distance: 0 ~ 3000
- Collision Preset: NoCollision
- Enable Nanite Support: ✅

### 0-1-7. import 검증 체크리스트
- [ ] 6개 SM (`SM_Var1_LOD0` ~ `SM_Var6_LOD0`) Content Browser에 보임
- [ ] 각 SM의 Nanite ON, LODGroup = Foliage
- [ ] 7개 텍스처 import 됨, sRGB 설정 표대로
- [ ] `MI_RibbonGrass` 생성, 6개 SM에 적용됨
- [ ] 빈 레벨에 SM 1개 드래그 → 잔디 보이고 알파 컷팅 정상
- [ ] PIE에서 캐릭터 가까이 가면 잎 휘어짐

---

## 1. MPC_Player 생성 (5분)

캐릭터의 월드 위치를 머티리얼이 읽을 수 있도록 전역 파라미터를 만든다.

1. Content Browser에서 `Content/Materials/` 폴더 생성 (없으면)
2. 폴더 안 우클릭 → `Materials → Material Parameter Collection`
3. 이름: `MPC_Player`
4. 더블클릭하여 열기
5. `Vector Parameters` 옆 `+` 클릭 → 새 항목 이름을 **`PlayerLocation`** 으로 (대소문자 정확히)
6. Default Value는 (0, 0, 0)으로 두기
7. Save

> 파라미터를 추가/이름 변경하면 **참조하는 머티리얼 모두 다시 컴파일** 됨. 이름은 이 단계에서 확정.

---

## 2. M_Grass_Bend 머티리얼 (가장 핵심, 1~2시간)

### 2-1. 머티리얼 생성
1. `Content/Materials/` 우클릭 → `Material`
2. 이름: `M_Grass_Bend`
3. 더블클릭하여 머티리얼 에디터 열기
4. 좌측 `Details` 패널에서 다음 설정:
   - `Shading Model` = `Default Lit`
   - `Two Sided` = ✅ 체크 (잔디 양면 보임)
   - `Use Material Attributes` = 끄기 (체크 해제)
   - `Num Customized UVs` = 0
5. (아직 저장하지 말고 노드 작업)

### 2-2. 잔디 메쉬에서 텍스처 가져오기

Quixel 잔디는 보통 다음 텍스처를 동봉:
- `T_..._Albedo` (BaseColor)
- `T_..._Normal` (Normal)
- `T_..._ORM` 또는 `T_..._AO_Roughness_Translucency` (R=AO, G=Roughness, B=Translucency 등)
- `T_..._Opacity` (잎 투명도) ← Masked 머티리얼이면 사용

**Blend Mode 결정:**
- 잎 가장자리가 알파로 잘려 있으면 → `Blend Mode = Masked`, 위 Detail 패널에 추가 옵션 등장. `Opacity Mask Clip Value = 0.33`
- 알파 없는 빌보드형 카드면 → `Blend Mode = Opaque`

### 2-3. 기본 머티리얼 그래프 (Albedo·Normal·Roughness)

머티리얼 에디터 그래프 빈 곳 우클릭으로 노드 추가. 단축키 ` (백틱) 도 사용 가능.

| 노드 | 이름/값 | 연결 |
|---|---|---|
| `Texture Sample` | T_..._Albedo 드래그 | RGB → Main의 `Base Color` |
| `Texture Sample` | T_..._Normal | RGB → Main의 `Normal` |
| `Texture Sample` | T_..._ORM (R채널) | R → Main의 `Ambient Occlusion`<br>G → Main의 `Roughness` |
| `Texture Sample` | T_..._Opacity (Masked일 때만) | R → Main의 `Opacity Mask` |

여기까지 컴파일(`Apply` 또는 `Save`) → Preview로 잎 모양 확인. 안 보이면 Two Sided 켜졌는지 확인.

### 2-4. 바람 효과 추가 (SimpleGrassWind)

`SimpleGrassWind` 노드 1개로 자연스러운 흔들림.

1. 그래프 빈 곳 우클릭 → 검색 `SimpleGrassWind` → 추가
2. SimpleGrassWind는 4개 입력:
   - `WindIntensity` (Scalar) — 흔들림 강도, 보통 0.1~0.5
   - `WindWeight` (Scalar) — 메쉬 상단으로 갈수록 강해지는 가중. 보통 1.0
   - `WindSpeed` (Scalar) — 빈도. 보통 1.0~3.0
   - `AdditionalWPO` (Vector3) — **필수 입력**. UE5 일부 버전에서 비워두면 컴파일 에러. 2-6에서 BendOffset을 여기 연결.

3. 파라미터를 노출하기 위해:
   - `Scalar Parameter` 노드 3개 추가, 이름 각각 `WindIntensity` / `WindWeight` / `WindSpeed`, 기본값 `0.3 / 1.0 / 1.5`
   - SimpleGrassWind의 해당 입력에 연결
4. **`AdditionalWPO`는 다음 단계(2-5)의 BendOffset을 받을 자리.** 일단 임시로 `Constant3Vector(0,0,0)` 연결해 컴파일 가능 상태로 둠 → 2-6에서 교체.

5. SimpleGrassWind 출력 → 다음 단계에서 Main의 WPO로 직결.

### 2-5. 캐릭터 휨 (★핵심 그래프)

플레이어 위치에서 멀어지는 방향으로 잎이 휜다. **여섯 단계로 빌드**:

#### A. MPC에서 PlayerLocation 가져오기
1. 우클릭 → `CollectionParameter` 노드 추가
2. Details 패널:
   - `Collection` = `MPC_Player`
   - `Parameter Name` = `PlayerLocation`
3. 출력은 RGB (Vector3)

#### B. 정점 → 플레이어 벡터 (★UE5 LWC 주의)

**중요:** UE5는 `WorldPosition`을 `LWCVector3`(Large World Coordinates)로 반환. `CollectionParameter`는 `float4`. 두 타입은 일반 `Subtract`로 빼면 **컴파일 에러** ("Arithmetic between LWCVector3 and float4 are undefined") 발생. **`LWCSubtract` 함수를 써야 함.**

1. `WorldPosition` 노드 추가 (검색: World Position)
2. `MakeLWCVector` 노드 추가 (검색: MakeLWCVector)
   - 입력 ← `CollectionParameter (PlayerLocation)`
3. `LWCSubtract` 함수 노드 추가 (검색: LWCSubtract)
   - `A` (LWC1) ← `WorldPosition`
   - `B` (LWC2) ← `MakeLWCVector` 출력
4. 결과는 **regular float3** (LWC 차감 결과). **플레이어에서 정점으로 향하는 벡터** = 밀어낼 방향
5. **일반 `Subtract` 노드 사용 금지** — UE5 LWC 도입 후 작동 안 함

#### C. 거리 계산 + 폴오프
1. 위 Subtract 결과를 `VectorLength` 노드(`Length`)에 연결 → 스칼라 거리
2. `Scalar Parameter` 추가, 이름 `BendRadius`, 기본값 `150`
3. `Divide` 노드: `A` = 거리, `B` = BendRadius → 0~∞ 비율
4. `OneMinus` 노드: `1 - x` → 가까우면 1, 멀면 음수
5. `Saturate` 노드 → 0~1로 클램프
6. (옵션) `Power` 노드: `Base` = 위 결과, `Exp` = `2.0` → 가까이서만 강하게 휨

이 결과 = **bend 강도 0~1**

#### D. 휨 방향 정규화
1. B단계 Subtract 결과를 `Normalize` 노드 통과 → 단위 벡터
2. **(권장 보정)** Y축 성분 죽이기: `Mask (R G B)` 또는 `ComponentMask` 노드로 `R, G` 만 남기고 `B=0`로 만든 다음 `Append`로 Vector3 재조립. 이유: 잎이 위/아래로 박히지 않게. 입문자는 일단 생략하고 결과 보고 추가
3. 결과 = **휨 방향 (단위 벡터)**

#### E. 정점 가중 마스크 (잎 끝만 휘게)
잔디 메쉬는 잎 뿌리(고정)와 끝(흔들림)을 구분해야 자연스럽다.

- **Quixel 메쉬:** `VertexColor` 노드의 `R` 또는 `G` 채널이 휨 마스크 (메쉬마다 다름, 보통 R)
- **임의 메쉬:** Object 로컬 Z를 마스크로
  - `LocalPosition` → `ComponentMask (B)` → 정점의 로컬 Z (잎 높이)
  - `Divide` 노드로 `BladeHeight`(예: 30) 나누기 → 0~1 정규화

이 가이드는 Quixel 기준이라 **`VertexColor.R`** 사용:
1. `VertexColor` 노드 추가
2. 출력에서 `R` 핀 사용 (없으면 ComponentMask R)

#### F. 합산 → BendOffset
1. `Multiply` 노드 1: A = 휨 방향(D), B = bend 강도(C 결과)
2. `Multiply` 노드 2: A = 위 결과, B = `VertexColor.R` (E 결과)
3. `Multiply` 노드 3: A = 위 결과, B = `Scalar Parameter "BendStrength"` (기본값 80)

이 결과 = **BendOffset Vector3**

### 2-6. 최종 WPO — BendOffset을 SimpleGrassWind에 합산

**개선된 방식:** 별도 `Add` 노드 만들지 않고 **BendOffset을 `SimpleGrassWind.AdditionalWPO`에 직접 연결**. SimpleGrassWind 내부에서 wind + bend 합산 처리. 더 깨끗하고 성능도 약간 더 좋음.

1. 2-4에서 `AdditionalWPO`에 임시로 연결한 `Constant3Vector(0,0,0)` **삭제**
2. 2-5 F의 마지막 `Multiply` (with BendStrength) 출력 → `SimpleGrassWind`의 `AdditionalWPO` 입력에 연결
3. `SimpleGrassWind` 출력 → Main 노드의 `World Position Offset` 핀

> **이전 가이드 버전의 `Add` 노드 방식은 폐기.** AdditionalWPO 사용이 정석.

### 2-7. 컴파일 & 테스트
1. 좌상단 `Apply` (체크마크) → `Save`
2. Content Browser에서 잔디 메쉬의 머티리얼 인스턴스 또는 메쉬 자체에 `M_Grass_Bend` 적용
   - 메쉬 우클릭 → `Edit` → Material Slot에 M_Grass_Bend 드래그
3. 빈 레벨에 잔디 메쉬 1개 드래그하여 배치
4. 머티리얼 에디터 미리보기에서는 휨 안 보임 (Player 위치가 0,0,0이라). 다음 단계에서 검증.

> **흔한 함정:** Apply 시 "Material has invalid output" 컴파일 에러 → 핀 연결 확인. WPO에 Vector3가 들어가야 함, Vector4 들어가면 Mask 필요.

---

## 3. BP_PrinceCharacter — MPC 송신 (5분, BP-only)

C++ 수정 없이 Blueprint 노드만으로 매 Tick MPC에 위치 전달.

1. Content Browser에서 `Content/Blueprints/BP_PrinceCharacter` 더블클릭
2. 상단 `Event Graph` 탭으로
3. **Event Tick** 노드를 그래프에 (이미 있으면 재사용)
4. 우클릭 → 검색 `Set Vector Parameter Value` → **"Set Vector Parameter Value (Material Parameter Collection)"** 선택
5. 노드 입력:
   - `Collection` = `MPC_Player` 드롭다운에서 선택
   - `Parameter Name` = `PlayerLocation` (정확한 대소문자)
   - `Parameter Value` ← `Get Actor Location` 노드 출력
   - World Context Object = `Self` (자동)
6. Event Tick 실행 핀 → Set Vector Parameter Value 실행 핀 연결
7. **Compile → Save**

> **성능 메모:** 매 Tick 호출이지만 MPC Set은 매우 가벼움 (uniform buffer 1개 update). 60fps 기준 무리 없음. 만약 Stat Unit에서 Game Thread 부담 보이면 다음 최적화 적용:
>
> - 멈춤 중에는 스킵: `Get Velocity` → `Vector Length` → `Branch (>0.01)` → True일 때만 Set
> - Frame skip: BP 변수 `int FrameCounter`, Tick마다 +1, `(FrameCounter % 2) == 0`일 때만 Set

---

## 4. Foliage Mode 페인팅 (15~30분)

### 4-1. Foliage Type 생성
1. Content Browser → `Content/` 안에 `Foliage` 폴더 생성
2. 폴더 안 우클릭 → `Foliage → Static Mesh Foliage`
3. 이름: `FT_Grass_Wild` (잔디 종류별로 1개씩 만들면 좋음)
4. 더블클릭하여 열기
5. `Mesh` 항목에 잔디 Static Mesh 드래그
6. 핵심 설정:
   - `Density / 1Kuu` = `2000` (시작값, 페인팅 시 조정)
   - `Random Yaw` = ✅
   - `Align to Normal` = ✅
   - `Random Pitch Angle` = `5` (살짝 기울임)
   - `Z Offset Min/Max` = `-2 / 2` (땅에 살짝 박히게)
   - `Cast Shadow` = ❌ 우선 끄기 (퍼포먼스, 나중에 조정)
   - `Cull Distance Min/Max` = `0 / 3000`
   - `Collision Preset` = `NoCollision` (캐릭터가 통과하게)
   - `Enable Nanite Support` = ✅ (UE5.1+, 메쉬가 Nanite면)

### 4-2. Foliage Mode로 페인팅
1. `L_PlanetTest` 레벨 열기
2. 상단 모드 드롭다운에서 `Foliage` 모드 선택 (또는 단축키 `Shift+4`)
3. 좌측 패널에 `FT_Grass_Wild` Foliage Type 드래그
4. 체크박스 켜기
5. **상단 옵션:**
   - Brush Size: `300` (시작값)
   - Paint Density: `0.5`
   - Erase Density: `0`
6. 행성 표면에 마우스 좌클릭 드래그하여 페인팅
   - 페인팅이 안 되면 `BP_Planet`의 메쉬 컴포넌트 → `Collision Complexity` = `Use Complex Collision As Simple` 확인 → 다시 시도
   - 또는 Foliage Type의 `Place on Static Meshes` ON 확인

> **구체 표면 정렬 안 맞으면:** Foliage Type의 `Align to Normal` ON, `Random Yaw` ON 재확인. `Z Offset Min/Max`을 음수로 살짝 조정 (-3 / -1).

### 4-3. 영역 한정
- 행성 전체 도배 X. **시작점 주변 + 1구역만** 페인팅 (Week 3에서 영역 확장)
- Erase는 `Shift + Click`

---

## 5. 검증 (10분)

### 5-1. 시각 검증
1. PIE (Play In Editor) 실행
2. 풀밭 안으로 걸어 들어감
3. **확인:** 캐릭터 주변 ~150cm 풀이 캐릭터 반대 방향으로 휘는가?
   - 안 휘면 → 다음 디버그
   - 너무 약하면 → 머티리얼 인스턴스에서 `BendStrength` 올리기
   - 휨 반경이 좁으면 → `BendRadius` 올리기 (예: 150 → 250)

### 5-2. 디버그 — 휨이 안 보일 때

| 증상 | 원인 후보 | 확인 |
|---|---|---|
| 풀이 가만히 있음 | MPC 송신 안 됨 | `BP_PrinceCharacter` Event Tick 노드 연결 확인. PIE 중 콘솔에서 `r.MaterialParameterCollection.LogValues 1` 후 `MPC_Player` 값 출력 |
| 풀이 미세하게 흔들림만 함 (바람만) | BendOffset이 WPO에 합산 안 됨 | M_Grass_Bend 그래프 검토 |
| 풀이 잘못된 방향으로 휨 | Subtract 순서 반대 (`PlayerLocation - WorldPosition`) | 노드 입력 A/B 바꾸기 |
| 풀 전체가 통째로 움직임 | VertexColor 마스크 미연결 | E단계 Multiply 누락 |
| 풀이 땅속으로 박힘 | 휨 방향 Z 성분이 너무 큼 | 2-5 D 단계 Y축(또는 Z축) 마스킹 추가 |

### 5-3. 퍼포먼스 검증
PIE 중 `~` (백틱) → 콘솔
- `stat fps` — 60+ 유지하는지
- `stat unit` — Frame, Game, Draw, GPU 시간 확인
- `stat foliage` — 인스턴스 수

기준:
- 인스턴스 1만 개 이하: 60fps 유지 (RTX 3060 기준)
- 인스턴스 5만 개 이상: Cull Distance 줄이기, Cast Shadow OFF 확인

### 5-4. 완료 기준
- [ ] 풀숲 안에서 30초 이상 보행, 끊김 없음
- [ ] 풀이 캐릭터에 따라 휨 (시각 확인)
- [ ] FPS 60+ (5천 인스턴스 기준)

---

## 6. 산출물 체크리스트

Week 1 끝나면 이 파일들이 존재해야 함:

- [ ] `Content/Materials/MPC_Player` (Vector Parameter `PlayerLocation`)
- [ ] `Content/Materials/M_Grass_Bend` (WPO + SimpleGrassWind + Bend graph)
- [ ] `Content/Foliage/FT_Grass_Wild` (또는 다른 이름)
- [ ] `Content/Megascans/.../Grass_*` (잔디 메쉬, M_Grass_Bend 적용됨)
- [ ] `Content/Blueprints/BP_PrinceCharacter` 변경 (Event Tick에 MPC Set 노드)
- [ ] `Content/Levels/L_PlanetTest` 변경 (풀숲 1구역 페인팅)

다음 단계: Week 2 — 상자 시스템 (`BP_Chest_Base` BP 작성).

---

## 부록 A — 파라미터 튜닝 권장값

머티리얼 인스턴스(`MI_Grass_Bend`) 만들어서 인스턴스에서 조절하면 컴파일 안 함:

| 파라미터 | 권장 범위 | 의미 |
|---|---|---|
| BendRadius | 100~250 | 휨 반경 (cm) |
| BendStrength | 50~150 | 휨 강도 (cm 단위 변위) |
| WindIntensity | 0.1~0.5 | 바람 강도 |
| WindWeight | 0.5~1.5 | 잎 끝 가중 |
| WindSpeed | 0.5~3.0 | 바람 빈도 |

머티리얼 인스턴스 생성: `M_Grass_Bend` 우클릭 → `Create Material Instance` → `MI_Grass_Bend`. Foliage Type의 메쉬 머티리얼 슬롯에 인스턴스 적용.

## 부록 B — Nanite 호환

UE5.3+ Nanite Foliage가 정식 지원. 조건:
- Static Mesh의 `Enable Nanite Support` ON
- 머티리얼이 Nanite 호환 (대부분 OK, Pixel Depth Offset 같은 일부 노드 제한)
- WPO Disable Distance 설정 가능 (먼 거리는 휨 비활성)

Quixel 잔디는 보통 Nanite 호환 메쉬로 import됨. 만약 메쉬 LOD가 잔뜩 있으면 Nanite ON으로 LOD 무시 가능.

## 부록 C — 콘솔 명령어 모음

| 명령 | 효과 |
|---|---|
| `stat fps` | FPS 표시 |
| `stat unit` | Frame/Game/Draw/GPU 시간 |
| `stat foliage` | Foliage 인스턴스 수 |
| `r.Foliage.MinimumScreenSize 0.000005` | 더 멀리도 컬링 안 함 (디버그용) |
| `show foliage` | 토글로 Foliage 끄기/켜기 |
| `r.MaterialParameterCollection.LogValues 1` | MPC 값 로그 출력 |
