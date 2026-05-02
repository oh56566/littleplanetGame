# 어린왕자 스핀오프 — 개발 로드맵

> 기획서 v0.1을 기반으로 한 시스템 단위 개발 계획. 각 Phase는 **그 자체로 검증 가능한 결과물**을 내고, 다음 Phase의 토대가 되도록 의도적으로 좁게 정의됐다.

---

## 0. 비전 압축

| 축 | 내용 |
|---|---|
| 장르 | 3D 액션 어드벤처 (싱글) |
| 톤 | 판타지 / 감성 / 액션 — 심각한 멜로물보다는 따뜻한 모험 + 잔잔한 충격 |
| 핵심 동사 | **걷기 · 만나기 · 줍기 · 사용하기 · 싸우기** |
| 핵심 루프 | 행성 도착 → 탐험 → 주민 이해 → 클리어 조건 → 약 → 거점 귀환 → 장미·여우 관계 변화 → 다음 행성 |
| 정체성 기둥 | (1) **작은 구 행성**의 곡면 위 액션 (2) **주민의 감정**을 이해해야 풀리는 구조 (3) **여우의 이중성** 복선 (4) 거점(고향 행성)의 점진적 회복 |

---

## 1. 현재 완료 상태

| 시스템 | 산출물 | 상태 |
|---|---|---|
| UE 5.7 C++ 프로젝트 골격 | `prince1` 모듈, GameMode | ✅ |
| 행성 액터 (반지름 가변) | `APlanetActor` | ✅ |
| 구 행성 보행 (CMC) | `UPlanetCharacterMovementComponent` (중력·바닥·랜딩 오버라이드) | ✅ |
| 3인칭 캐릭터 + 카메라 + 점프 | `APrinceCharacter` | ✅ |
| 표면 정렬 소품 베이스 | `APlanetSurfaceActor` | ✅ |

이로써 "**행성 위 어떤 위치에든 캐릭터를 두고 걷고 점프할 수 있고, 소품을 클릭만으로 자연스럽게 배치할 수 있다.**" 모든 후속 시스템은 이 위에 얹는다.

---

## 2. 시스템 의존성 그래프

```
Foundation (DONE)
   │
   ├─→ Multi-Planet & Travel ──┐
   │                            │
   ├─→ Interaction & Items ────┼──→ Planet Clear Conditions ──┐
   │                            │                              │
   ├─→ NPC & Dialogue ─────────┼──→ Hub Progression (Rose) ──┤
   │                            │                              │
   ├─→ Combat ─────────────────┼──→ Emotion Monsters ────────┼──→ Boss System
   │                            │                              │       │
   │                            └──→ Companion AI (Fox) ───────┤       │
   │                                                            ↓       ↓
   └────────────────────────────────────────────→ Story / Chapters / Events
                                                                ↓
                                                        Save · Audio · Polish
```

가장 강한 의존성: **Interaction & Items**는 이후 거의 모든 게임플레이의 어휘다. **Combat**과 **NPC/Dialogue**는 병렬로 진행 가능. **Boss/Companion/Story**는 위 시스템이 안정된 후 합성.

---

## 3. Phase별 개발 계획

각 Phase는 1~3주 단위로 끊어 검증 가능한 마일스톤을 만든다.

### Phase 1 — 다중 행성 + 거점/탐험 구조
**목표**: 두 개 이상의 행성 사이를 이동하고, "거점 행성(고향)"과 "탐험 행성"의 구분을 코드/데이터로 표현.

- [ ] `APlanetActor`에 `EPlanetRole` (Home/Exploration), `FName PlanetID`, `FText DisplayName` 노출
- [ ] **행성 간 이동 방식 결정**: 별도 우주 공간 비행 vs. 즉시 워프(포털 액터). 권장: **포털 워프** (스코프 작음, 기획서에 우주 공간 언급 없음)
- [ ] `APlanetPortalActor`: 표면에 두는 표면-정렬 액터, 상호작용 시 대상 행성으로 캐릭터 텔레포트(중력 자동 전환은 `UPlanetCharacterMovementComponent`가 이미 처리하므로 단순 이동만)
- [ ] `UPrinceWorldSubsystem` (게임 인스턴스 서브시스템): 현재 행성 추적, 행성 목록 관리
- [ ] 테스트 레벨에 거점 행성 1개 + 탐험 행성 1개 배치, 포털 왕복 검증

**검증**: 거점 ↔ 탐험 행성을 포털로 왕복할 때 중력·카메라·바닥 정상.

### Phase 2 — 상호작용 & 아이템 시스템
**목표**: 게임의 핵심 어휘 정의. "줍고, 들고, 놓고, 사용한다."

- [ ] `IInteractable` 인터페이스 (`UFUNCTION(BlueprintNativeEvent)` 패턴): `OnInteract`, `GetPromptText`
- [ ] `APrinceCharacter`에 인터랙트 트레이스 (전방 짧은 구 트레이스, 매 틱), 가장 가까운 IInteractable 캐싱
- [ ] `IA_Interact` (E키) → 캐싱된 대상 호출
- [ ] `UInteractPromptWidget` (UMG): 화면 중앙에 "[E] {prompt}"
- [ ] `UItemDataAsset` (`UPrimaryDataAsset`): 아이콘, 이름, 설명, ID
- [ ] `APickupActor : APlanetSurfaceActor` + `IInteractable`: 줍기 시 인벤토리에 추가
- [ ] `UInventoryComponent`: TArray<UItemDataAsset*>, `AddItem`/`RemoveItem`/`HasItem`/이벤트
- [ ] `APlacementSlotActor`: 특정 아이템을 받을 수 있는 슬롯 (예: 왕좌 옆 빈자리). 배치 시 `OnSlotFilled` 이벤트 발화

**검증**: 행성 위에 떨어진 [의자] 픽업 → 인벤토리에 추가 → 슬롯 옆에서 E → 슬롯에 의자 메쉬 표시 + 이벤트 발화.

### Phase 3 — NPC & 대화 시스템
**목표**: 주민과 장미와 대화한다.

- [ ] `ANpcActor : APlanetSurfaceActor` + `IInteractable`: 표면에 자동 정렬되는 NPC
- [ ] `UDialogueDataAsset` 또는 `DataTable`: 노드 = (스피커, 본문, 다음 노드 ID 또는 선택지[])
- [ ] `UDialogueComponent`: 현재 노드, 진행/선택 메서드
- [ ] `UDialogueWidget` (UMG): 박스 + 텍스트 페이드인 + 스페이스로 다음
- [ ] `UDialogueSubsystem` (UWorldSubsystem): 활성 대화 1개 관리, 캐릭터 입력 차단
- [ ] 거점에 `BP_Rose` (특수 NPC, 장미 회복 단계에 따라 대사 다름) — Phase 8에서 정교화

**검증**: NPC 옆에서 E → 대화 박스 출현, 스페이스로 진행, 끝나면 닫힘. 캐릭터 이동 일시 차단.

### Phase 4 — 전투 (액션 기본기)
**목표**: 공격 입력 → 히트 → 데미지 → 죽음의 사이클이 동작.

- [ ] `UHealthComponent`: 현재/최대 HP, `ApplyDamage`/`IsDead`/이벤트
- [ ] `IDamageable` 인터페이스: `TakeDamageWith(FDamageInfo)` (방향·세기·태그 포함)
- [ ] `IA_Attack` (좌클릭) → `APrinceCharacter`의 가벼운 콤보 (3히트 정도)
- [ ] 공격 판정: 히트박스 컴포넌트 (`UCapsuleComponent` 한 프레임 활성), 또는 단순한 짧은 트레이스. 권장 시작은 **트레이스** (애니메이션 의존 최소화)
- [ ] 피격 반응: HitReact 몽타주 슬롯, 짧은 무적 시간
- [ ] 공격 입력 시 캡슐을 카메라 방향으로 잠시 회전 (CMC의 기존 회전 로직과 협조)
- [ ] HUD: 화면 좌상단 HP 바 (`UPlayerHudWidget`)

**검증**: 더미 액터(`AHitDummyActor`)를 행성에 두고 공격 → 데미지 누적 → 0이 되면 사라짐.

### Phase 5 — 감정 몬스터 시스템
**목표**: 행성마다 다른 외형/행동의 적. "이 적이 누구의 어떤 감정인가"를 시각/행동으로 전달.

- [ ] `AEnemyBase : ACharacter` (또는 표면 정렬용 별도 베이스) + `UHealthComponent` + `IDamageable`
- [ ] **AI**: UE의 Behavior Tree + Blackboard. 단순 트리 — Idle/Patrol → Detect Player(시야) → Chase → Attack → Cooldown
- [ ] `UEnemyDefinitionDataAsset`: 메쉬·머티리얼·스피드·HP·공격력·드롭 아이템
- [ ] `EmotionTag` (예: Loneliness, Envy, Anger) → 머티리얼 파라미터/행동 바리에이션 연결
- [ ] 사망 시 `UItemDataAsset` 드롭 (Phase 2의 `APickupActor` 스폰)
- [ ] 행성에 스포너(`AEnemySpawnerActor`) — 표면 정렬 + 영역 내 랜덤 배치

**검증**: 한 행성에 "외로움"이라는 감정 몬스터 3마리 스폰 → 플레이어 추적 → 공격 → 처치 → 의자 드롭.

### Phase 6 — 보스 + 행성 클리어 조건
**목표**: 단순 처치가 아닌 "주민의 진짜 욕구를 이해"하는 클리어.

- [ ] `APlanetClearConditionActor`: 행성 단위로 1개. 조건 평가 (예: "특정 슬롯이 채워졌는가"), 충족 시 보스 출현 또는 약 드롭
- [ ] 약: `UItemDataAsset`의 특수 카테고리 (`bIsMedicine = true`)
- [ ] `ABossActor : AEnemyBase` — 페이즈 전환, 패턴 (텔레그래프-이동-공격), 소환 가능
- [ ] 보스 처치 → 약 드롭 → 거점 포털 활성화

**검증**: 왕의 행성 시나리오 — 의자를 슬롯에 놓음 → 보스 등장 → 처치 → 약 획득 → 거점 귀환.

### Phase 7 — 동반자 AI (여우)
**목표**: 여우가 플레이어를 따라다니고 가벼운 보조를 한다. 후반 복선의 그릇.

- [ ] `AFoxCompanionActor : ACharacter` (행성 보행 적용을 위해 같은 CMC 사용)
- [ ] `UCompanionFollowComponent`: 플레이어 뒤 일정 거리 유지, 계단/곡면에서 끊기지 않게 NavMesh 또는 직접 트레이싱
- [ ] 가벼운 전투 보조: 가까운 적 1마리에게 짧은 공격 트리거 (옵션, 후순위)
- [ ] **이중인격 상태머신**: `EFoxState` (Normal/Spaced-out/Other-self/Aware), 챕터 진행도에 따라 상태 변경
- [ ] 상태별 대화 트리거 (Phase 3 시스템 재사용)

**검증**: 거점에서 여우와 합류 → 탐험 행성에서 따라옴 → 챕터 플래그 전진 시 멍해지는 짧은 컷 출현.

### Phase 8 — 거점 진행 (장미 상태 / 잠금해제)
**목표**: 약 누적 → 장미 상태 단계 변경 → 캐릭터 능력/대화 잠금해제.

- [ ] `URoseStateComponent` (장미 NPC에 부착): `MedicineCount`, `ERoseHealth` 단계, 단계 변경 이벤트
- [ ] `UProgressionDataAsset`: 단계별 잠금해제 (스킬 ID, 무기 업그레이드 등)
- [ ] `UPrincePlayerComponent`: 잠금해제된 능력 셋, 능력 사용 게이팅
- [ ] 약 사용: 거점에서 장미와 상호작용 → 인벤토리에서 약 소비 → 단계 진행
- [ ] 단계별 장미 머티리얼/메쉬/대사 변경

**검증**: 약 1개를 가지고 거점 귀환 → 장미와 상호작용 → 회복 단계 진행 → 새 능력 1개 사용 가능.

### Phase 9 — 스토리 / 챕터 / 이벤트 시스템
**목표**: 챕터 진행도가 한 곳에서 관리되고, 모든 시스템이 거기서 상태를 읽음.

- [ ] `UStorySubsystem` (UGameInstanceSubsystem): `EChapter`, `TSet<FName> Flags`
- [ ] **이벤트 트리거 액터**들: `ATriggerVolume_StoryFlag`, `ADialogueOnFlagActor` 등
- [ ] 여우의 이상 징후를 챕터별 이벤트로 데이터화 (3-3 표를 그대로 데이터)
- [ ] **클라이맥스 보스**: 여우 본체와 다른 자아를 분리하는 특수 보스전 (Phase 6의 보스 시스템 + 새 컴포넌트)
- [ ] 엔딩 시퀀스 (간단한 카메라 이동 + 디졸브 + 크레딧)

**검증**: 약 N개를 모으면 마지막 행성 포털 활성화 → 보스 처치 → 엔딩 트리거.

### Phase 10 — 폴리시 (Save / Audio / VFX / Tutorial / UI)
**목표**: 플레이 가능을 넘어 "전달 가능"한 빌드.

- [ ] `USaveGame` 직렬화 — 인벤토리·진행도·현재 행성·장미 단계
- [ ] 메인 메뉴 / 일시정지 / 옵션
- [ ] 사운드: BGM(거점/탐험/전투 3종), 발걸음, UI 클릭, 약 획득 효과음
- [ ] 파티클: 공격 히트, 약 드롭, 포털, 장미 회복
- [ ] 첫 행성에 가이드 UI (조작법 노출)
- [ ] 컷씬: 프롤로그(장미 발병 → 기록 발견 → 여우 만남), 엔딩

---

## 4. 권장 다음 단계 — Vertical Slice 정의

지금 가장 가성비 높은 진행은 **"거점 1개 + 탐험 행성 1개로 1회 루프 완주"** 를 만드는 것이다. 이게 동작하면 나머지 행성/스토리는 콘텐츠 추가일 뿐.

**Vertical Slice 범위 (예상 4~6주)**
1. **Phase 1** 다중 행성 + 포털 (1주)
2. **Phase 2** 상호작용 + 픽업 + 슬롯 (1주)
3. **Phase 3** NPC + 대화 — 최소 기능 (1주)
4. **Phase 4** 전투 — 가벼운 콤보 + HP/HUD (1주)
5. **Phase 5** 감정 몬스터 — 1종 (3일)
6. **Phase 6** 보스 — 매우 단순한 1패턴 보스 (3일)
7. **Phase 8** 거점 진행 — 1단계만 (3일)

이 슬라이스에서 의도적으로 **빼는 것**: 여우, 컷씬, 저장, 사운드, 다중 챕터. 메커니즘이 재미있는지부터 검증한다.

**다음 작업 후보 (당장 시작할 수 있는 것)**:
- A. **포털 + 다중 행성** (`Phase 1`) — 시각적으로 큰 진보
- B. **상호작용 + 픽업** (`Phase 2`) — 게임플레이 어휘 확립, 향후 모든 시스템의 토대
- C. **전투 기본** (`Phase 4`) — 액션 게임의 손맛 검증

권장 순서: **B → A → 그 다음 병렬로 C/D**. 상호작용은 NPC·픽업·슬롯·포털 모두에 쓰여서 가장 먼저 만들 가치가 있다.

---

## 5. 의사결정이 필요한 사항

| 항목 | 옵션 | 의견 |
|---|---|---|
| 행성 간 이동 | 포털 워프 / 우주 비행 | **포털** — 기획서에 비행 언급 없음, 스코프 큼 |
| 데이터 구동 정도 | 하드코딩 / DataAsset / DataTable | **DataAsset 우선** — 적·아이템·대화 모두 |
| 캐릭터 메쉬 | 임시 캡슐 / UE5 마네킹 / 외부 에셋 | **마네킹 → 외부 에셋**. 시스템 검증까진 마네킹 |
| 애니메이션 | 키프레임 / Mixamo / 외주 | 프로토타입은 **Mixamo 무료**로 충분 |
| 사운드/음악 | 무료 라이브러리 / 작곡 의뢰 | 슬라이스까진 **무료**, 정식 빌드는 의뢰 |
| 저장 시스템 | UE SaveGame / 직접 JSON | **UE SaveGame** — 충분 |
| 멀티플레이어 | 싱글 / 협력 | 기획서가 싱글이라 **싱글 고정** |

---

## 6. 외부 자산 필요 목록 (조달 계획용)

- 캐릭터: 왕자, 여우, 장미, 행성별 주민 1~6
- 적: 감정 몬스터 (행성당 1~2종), 보스 (행성당 1)
- 환경: 행성 메쉬 셋(지형/식생/구조물), 포털, 픽업 아이콘 셋
- UI: 폰트, 다이얼로그 박스, HP 바, 인벤토리 아이콘
- 사운드: BGM 3~5트랙, SFX 30~50개
- 컷씬: 프롤로그·엔딩 정적 일러스트 또는 짧은 영상

---

## 7. 위험 요소

1. **표면 클리어 조건의 퍼즐 디자인** — 시스템보다 디자인이 어렵다. 행성 1개를 끝까지 만들어 보면서 패턴 추출
2. **곡면 위 NavMesh** — UE의 NavMesh는 평평한 표면 가정. 적 AI 이동을 NavMesh 없이 직접 처리해야 할 수도 (현재 플레이어 CMC처럼)
3. **여우의 동반자 AI 끊김** — 곡면 추적 + 플레이어 점프 시 갭. Phase 7에서 별도 검증 시간 필요
4. **데이터 양 폭증** — 행성 6~7개 × (적 + 주민 + 대화 + 클리어 조건) = 수백 개 에셋. Phase 10 전에 데이터 관리 워크플로(Editor Utility) 한 번 정비

---

## 8. 다음 갱신
이 로드맵은 살아있는 문서. Phase 완료 시마다 학습 사항을 반영해 다음 Phase의 범위를 좁힌다.

— v0.1 (Foundation 완료 직후 작성)
