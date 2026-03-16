0# Final Minutes

> 10분 안에 살아남아라 — UE5 기반 서바이벌 좀비 슈터

[![시연 영상](https://img.youtube.com/vi/jP3IUdCeRC0/0.jpg)](https://www.youtube.com/watch?v=jP3IUdCeRC0)

**내일배움캠프 Unreal Engine 과정 | 13조 고양이사랑단**

**프로젝트 기간:** 2026.02.05 ~ 2026.03.05 (4주, 약 20 워킹데이)

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine%205.5-313131?style=for-the-badge&logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![GAS](https://img.shields.io/badge/Gameplay%20Ability%20System-313131?style=for-the-badge)
![Enhanced Input](https://img.shields.io/badge/Enhanced%20Input-313131?style=for-the-badge)
![AI](https://img.shields.io/badge/AI%20Behavior%20Tree-313131?style=for-the-badge)
![Niagara](https://img.shields.io/badge/Niagara%20VFX-313131?style=for-the-badge)

---

## 팀원 및 담당 기능

| 이름 | 역할 | 담당 시스템 |
|------|------|------------|
| 허태린 | 팀장 | 게임모드 (GameMode, GameState, 타이머, 세이브 시스템) |
| 최준혁 | 부팀장 | 웨폰 (무기 데이터 에셋, 전투 컴포넌트, 투사체) |
| 박둘내 | 팀원 | UI (HUD, 인벤토리 위젯, 플레이어 상태 표시) |
| 유시환 | 팀원 | 몬스터 (AI, 비헤이비어 트리, 웨이브 스폰) |
| 홍종규 | 팀원 | 플레이어 (캐릭터, GAS 어빌리티, 이동/스태미나) |

---

## 프로젝트 구조

```
Source/FinalMinutes/
├── AbilitySystem/    # GAS 어빌리티, 어트리뷰트, 애님 노티파이
├── Character/        # 플레이어, 적 캐릭터, CombatComponent
├── Components/       # InventoryComponent
├── Controller/       # AI/Player/Zombie 컨트롤러
├── Core/             # GameInstance, SaveGame
├── Framework/        # GameMode, GameState
├── Interfaces/       # Damageable, Interactable
├── Items/            # 아이템, 무기, 투사체, 스폰
├── Monster/          # 몬스터 캐릭터, 웨이브 스폰
├── Subsystems/       # 4개 서브시스템
├── Task/             # BT 태스크/서비스
└── UI/               # HUD, 위젯
```

---

## 트러블 슈팅

### [유시환] 아이템 랜덤 스폰 시스템
**Problem:** BoxCollision 내 무작위 좌표로 아이템 스폰 시, 벽 내부나 공중에 생성되는 엣지 케이스 발생
**Cause:** Z축 검증 없이 랜덤 좌표만 사용하여 NavMesh 밖 위치에 스폰
**Solution:** NavMesh 검증 + 바닥면 Line Trace + 겹침 방지 로직 추가로 유효한 위치에만 스폰되도록 개선

### [허태린] [TODO: 작성 필요]
**Problem:** [TODO: 작성 필요]
**Cause:** [TODO: 작성 필요]
**Solution:** [TODO: 작성 필요]

### [최준혁] [TODO: 작성 필요]
**Problem:** [TODO: 작성 필요]
**Cause:** [TODO: 작성 필요]
**Solution:** [TODO: 작성 필요]

### [박둘내] [TODO: 작성 필요]
**Problem:** [TODO: 작성 필요]
**Cause:** [TODO: 작성 필요]
**Solution:** [TODO: 작성 필요]

### [홍종규] [TODO: 작성 필요]
**Problem:** [TODO: 작성 필요]
**Cause:** [TODO: 작성 필요]
**Solution:** [TODO: 작성 필요]

---

## 기술 아키텍처

### 게임 프레임워크
- GameMode → GameState → PlayerController 구조로 게임 흐름 제어
- 10분 타이머 기반 게임 루프 (생존 시 GameClear, 사망 시 GameOver)
- BGM 3단계 페이즈 전환으로 시간 경과에 따른 긴장감 연출
- GameInstanceSubsystem 기반 세이브/로드 시스템

### 전투 시스템 (GAS)
- 모든 플레이어 액션(공격, 재장전, 구르기, 달리기 등)을 GameplayAbility로 구현
- CharacterAttributeSet으로 체력, 스태미나, 공격력, 방어력, 이동속도 관리
- GameplayTag로 어빌리티 상태 정의 및 이동 제약 조건 제어

### 무기 시스템 (Data-Driven)
- WeaponDataAsset + DataTable 연동으로 하드코딩 방지
- CombatComponent가 주무기/보조무기 슬롯, 반동, 탄약을 독립적으로 관리
- 비동기 메시 로딩(Streamable Handle)으로 무기 전환 시 성능 최적화

### AI & 스폰 시스템
- Behavior Tree + AI Perception 기반 몬스터 AI
- 웨이브 기반 스폰: 시간에 따라 스폰 강도 및 몬스터 수 증가
- 부위별 데미지 배율 (머리/몸통/사지)로 정밀 사격 보상

### 서브시스템 구조
- GameInstanceSubsystem으로 모듈 간 결합도 최소화
- WeaponRegistrySubsystem: GameplayTag 기반 무기 데이터 캐싱/조회
- SaveSubsystem: 킬 카운트, 최고 생존 시간, 인벤토리 영속화
- ItemDataSubsystem / SoundSubsystem: 아이템 데이터 및 오디오 중앙 관리
