# Final Minutes

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
| [허태린](https://github.com/ouranos0902) | 팀장 | 게임모드 (GameMode, GameState, 타이머, 세이브 시스템) |
| [최준혁](https://github.com/OptimalTime99) | 부팀장 | 웨폰 (무기 데이터 에셋, 전투 컴포넌트, 투사체) |
| [박둘내](https://github.com/Einside0227) | 팀원 | UI (HUD, 인벤토리 위젯, 플레이어 상태 표시) |
| [유시환](https://github.com/shy3939/shy3939.git) | 팀원 | 몬스터 (AI, 비헤이비어 트리, 웨이브 스폰) |
| [홍종규](https://github.com/JongKyuHong) | 팀원 | 플레이어 (캐릭터, GAS 어빌리티, 이동/스태미나) |

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

### [유시환]
#### 1. 아이템 랜덤 스폰 시스템
- **Problem:** BoxCollision 내 무작위 좌표로 아이템 스폰 시, 벽 내부나 공중에 생성되는 엣지 케이스 발생  
- **Cause:** Z축 검증 없이 랜덤 좌표만 사용하여 NavMesh 밖 위치에 스폰  
- **Solution:** NavMesh 검증 + 바닥면 Line Trace + 겹침 방지 로직 추가로 유효한 위치에만 스폰되도록 개선

---

### [허태린] 세이브/사운드 서브 시스템
#### 1. GAS AttributeSet 기반 무기 탄약 세이브/로드 데이터 동기화 구현
- **Problem:** 세이브 데이터 로드 시  무기 탄약이 초기화 되는 문제
- **Cause:** 실시간으로 Gameplay Ability System(GAS)의 UWeaponAttributeSet에서 탄약 수치를 관리하므로, 액터 변수(CurrentAmmoCount)만 저장하면 최신 수치가 누락됨.
- **Solution:**
<img width="666" height="612" alt="Image" src="https://github.com/user-attachments/assets/5c602d5b-f51f-41e2-a795-ab6206bcb483" />  

CombatComp->GetSpawnedWeapons() 맵을 전체 순회하여 플레이어가 한 번이라도 소지했던 모든 무기 인스턴스를 누락 없이 확인

#### 2. 게임 페이즈별 BGM 재생 시스템 구축 및 에셋 참조 안정화
- **Problem:** 게임이 시작되어도 BGM이 나오지 않거나, 시간이 지나도 다음 페이즈의 음악으로 전환되지 않는 현상
- **Cause:** SoundSubsystem에서 재생할 BGM_Phase1, BGM_Phase2, BGM_Phase3 등 3단계의 BGM 에셋이 코드나 블루프린트 내부에서 제대로 할당되지 않아 Null 참조 상태였기 때문
- **Solution:** 블루프린트 내부 로직만으로 사운드를 로드하는 대신, BP_GameState의 디테일 창(Details Panel)에서 BGM_Phase1, 2, 3 에셋(.WAV)을 직접 할당하여 엔진에서 참조할 수 있도록 함.

---

### [최준혁] 무기/전투 컴포넌트, 투사체 액터
#### 1. 비동기 로딩 및 객체 풀링을 통한 무기 교체 프레임 드랍 최적화
- **Problem**: 런타임 중 플레이어가 새로운 무기를 스폰하거나 교체할 때 메인 스레드가 블로킹되어 심각한 프레임 드랍이 발생함.
- **Cause**: 무기를 장착할 때마다 용량이 큰 스켈레탈 메시 에셋을 동기적으로 로드하고 새로운 액터를 메모리에 할당하기 때문임.
- **Solution**: `FStreamableManager`를 도입하여 무기 메시를 백그라운드에서 비동기로 로드하도록 구조를 개선함. 메시 로딩이 완료되기 전에도 무기 로직은 정상적으로 초기화되도록 분리함. 
```cpp
void AWeaponBase::StartAsyncMeshLoad(FGameplayTag InWeaponTag)
{
    FSoftObjectPath MeshPath = CurrentDataAsset->WeaponData.WeaponMesh.ToSoftObjectPath();
    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

    // 진행 중인 로딩 취소 후 새로 시작
    if (WeaponMeshLoadHandle.IsValid() && WeaponMeshLoadHandle->IsLoadingInProgress())
    {
        WeaponMeshLoadHandle->CancelHandle();
    }

    // RequestAsyncLoad를 통한 백그라운드 비동기 로딩 요청
    WeaponMeshLoadHandle = Streamable.RequestAsyncLoad(
        MeshPath,
        FStreamableDelegate::CreateUObject(this, &AWeaponBase::OnWeaponMeshLoaded, InWeaponTag)
    );
}
```

또한, 장착 해제된 무기 액터를 파괴하는 대신 `SpawnedWeapons` 맵에 캐싱하고 렌더링만 숨김 처리하는 방식을 적용하여, 빈번한 스폰 및 파괴로 인한 오버헤드를 제거함.
```cpp
// UCombatComponent::EquipWeapon 내부

// --- 1) 태그로 이미 스폰된 무기인지 확인 ---
AWeaponBase* WeaponToEquip = nullptr;
const bool bAlreadySpawned = SpawnedWeapons.Contains(Tag);

if (bAlreadySpawned)
{
    // 이미 메모리에 할당된 무기가 있다면 캐싱된 액터를 재사용
    WeaponToEquip = SpawnedWeapons[Tag];
}
else
{
    // --- 2) 처음 보는 무기면 스폰 후 맵에 캐싱 ---
    WeaponToEquip = GetWorld()->SpawnActor<AWeaponBase>(...);
    SpawnedWeapons.Add(Tag, WeaponToEquip);
    
    WeaponToEquip->InitializeWeapon(Tag, OwnerCharacter);
}
```

#### 2. 총알의 관통 및 자가 피격 버그 해결
- **Problem**: 탄속이 매우 빠른 투사체가 얇은 벽이나 빠르게 이동하는 적을 통과해버리거나, 발사 직후 플레이어 자신의 콜리전에 부딪혀 즉시 피격 판정이 발생하는 문제 발생.
- **Cause**: 물리 엔진의 이산적 충돌 검사 프레임 간격 사이에서 투사체가 목표물을 완전히 지나쳐버렸으며, 발사 스폰 위치가 플레이어의 캡슐 콜리전과 겹쳐 있었기 때문임.
- **Solution**: 투사체의 `USphereComponent` 설정에서 연속 충돌 검사(CCD) 옵션을 활성화하여 고속 이동 시에도 프레임 간 관통 문제를 방지함. 자가 피격 문제의 경우 투사체 초기화 코드에서 `IgnoreActorWhenMoving()` API를 호출하여 발사 주체와의 물리적 충돌만 선택적으로 무시하도록 안전장치를 구현함.
```cpp
AProjectileBullet::AProjectileBullet()
{
    // ... (다른 초기화 코드 생략) ...

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(5.0f);
    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));

    // 핵심: 빠른 탄속에서 벽이나 적을 뚫고 지나가는 '터널링' 현상 방지
    // 연속 충돌 감지(CCD)를 활성화하여 프레임 사이의 궤적까지 물리 연산에 포함
    CollisionComp->BodyInstance.bUseCCD = true;

    // ...
}
```
```cpp
void AProjectileBullet::InitializeProjectile(const FGameplayEffectSpecHandle& InSpecHandle, float InSpeed)
{
    // ... (속도 설정 등 생략) ...

    // 핵심: 발사 즉시 자기 자신(플레이어)의 콜리전과 부딪혀 소멸하는 현상 방지
    AActor* MyInstigator = GetInstigator();
    if (MyInstigator)
    {
        // 물리 이동 시 Instigator(발사자)와의 충돌만 선택적으로 무시하도록 안전장치 적용
        CollisionComp->IgnoreActorWhenMoving(MyInstigator, true);
    }
}
```

---

### [박둘내] 아이템
#### 1. 공간 검증을 통한 아이템 비정상 스폰 이슈 해결
- **Problem:** BoxCollision 내 무작위 좌표로 아이템 스폰 시, 벽 내부나 공중에 생성되는 엣지 케이스 발생  
- **Cause:** Z축 검증 없이 랜덤 좌표만 사용하여 NavMesh 밖 위치에 스폰  
- **Solution:** NavMesh 검증 + 바닥면 Line Trace + 겹침 방지 로직 추가로 유효한 위치에만 스폰되도록 개선  

#### 2. 액터 타입 검증을 통한 Line Trace 참조 크래시 방지
- **Problem:** 아이템이 아닌 지형지물에 Line Trace가 닿았을 때, 게임이 크래시 나는 문제 발생  
- **Cause:** Line Trace에 맞은 액터를 모두 아이템으로 가정하고 ItemID에 접근했지만, 지형지물 액터에는 해당 값이 없어 예외 상황 발생  
- **Solution:** Line Trace에 맞은 액터가 실제 아이템인지 먼저 검사하고, ItemID가 유효한 경우에만 상호작용 로직이 실행되도록 조건문을 추가  

---

### [홍종규] 🛠 사망 판정
> [블로그](https://velog.io/@kyu_/Day-60-Death-%EB%A7%8C%EB%93%A4%EA%B8%B0)

#### 1. 페이로드 연동 방식 변경에 따른 사망 어빌리티 미발동 이슈 해결
- **Problem:** Payload 방식으로 변경 후 `GA_Death`가 실행되지 않는 문제
- **Cause:** `TryActivateAbilitiesByTag` 방식과 달리, `SendGameplayEventToActor`로 Payload를 함께 전달할 때는 Ability Triggers 설정이 필수인데 안해주었다.
- **Solution:**
  - BP_GA_Death의 Ability Triggers
    - Trigger Tag: Event.Montage.Death
    - Trigger Source : Gameplay Event로 변경해서 해결

![AbilityTrigger](https://raw.githubusercontent.com/JongKyuHong/image/main/AbilityTrigger.png)

#### 2. 사망 어빌리티 실행 시 사망 태그 누락 문제 수정
- **Problem:** GE_Death 적용 후 `State.Player.Death`태그를 실제로 보유하지 않는 문제
- **Cause:** GA_Death에서 DeathEffectClass에 BP_GE_Death를 지정만 하고, ApplyGameplayEffectSpecToOwner로 실제 적용(Apply) 하는 코드가 없었음
- **Solution:** 
  - `GA_Death`의 ActivateAbility에서 GE를 생성하고 자신에게 직접 적용하도록 추가

![GA_DeathCode](https://raw.githubusercontent.com/JongKyuHong/image/main/GA_DeathCode.png)

#### 3. 태그 기반 상태 제어를 통한 사망 후 자세 변경(앉기/엎드리기) 버그 픽스
- **Problem:** 사망 후에도 앉기/엎드리기 가능한 문제
- **Cause:** `GA_Crouch`, `GA_Prone`의 실행 차단 조건에 `State.Player.Death` 태그가 없었음
- **Solution:** GA_Prone, GA_Crouch의 `ActivationBlockedTags`에 Death 태그 추가

![GA_BlockTag](https://raw.githubusercontent.com/JongKyuHong/image/main/GA_BlockTag.png)


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
