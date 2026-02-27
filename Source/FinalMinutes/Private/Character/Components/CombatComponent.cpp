// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Components/CombatComponent.h"

// 엔진 및 프레임워크 헤더
#include "AbilitySystemComponent.h"
#include "Character/Player/PlayerCharacter.h"
#include "GameFramework/PlayerController.h"

// 무기 시스템 관련 헤더
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Items/Weapons/WeaponBase.h"
#include "Items/Weapons/WeaponDataAsset.h"
#include "Subsystems/WeaponRegistrySubsystem.h"

UCombatComponent::UCombatComponent()
{
    // 반동 보간을 위해 Tick사용
    PrimaryComponentTick.bCanEverTick = true;
    
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    // 소유자 캐릭터를 캐싱하여 이후 매 프레임 형변환 비용을 줄입니다.
    OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
}

/** * [조회] 슬롯에 따른 무기 포인터를 반환합니다. 
 */
AWeaponBase* UCombatComponent::GetWeaponBySlot(const EWeaponSlot Slot) const
{
    switch (Slot)
    {
    case EWeaponSlot::Primary: return PrimaryWeapon;
    case EWeaponSlot::Secondary: return SecondaryWeapon;
    default: return nullptr;
    }
}

/** * [핵심 로직 1] 무기 장착 (Equip)
 * 새로운 무기를 스폰하고 알맞은 슬롯에 배치합니다.
 */
void UCombatComponent::EquipWeapon(FGameplayTag Tag)
{
    if (!OwnerCharacter) OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
    if (!OwnerCharacter || !Tag.IsValid()) return;

    UWeaponRegistrySubsystem* Registry = GetWorld()->GetGameInstance()->GetSubsystem<UWeaponRegistrySubsystem>();
    UWeaponDataAsset* WeaponDataAsset = Registry ? Registry->GetWeaponDataByTag(Tag) : nullptr;
    if (!WeaponDataAsset) return;

    EWeaponSlot TargetSlot = WeaponDataAsset->WeaponData.DefaultSlot;
    UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(GetOwnerASC()->GetSet<UWeaponAttributeSet>());

    // --- 1. 기존 무기 제거 (있을 때만 수행) ---
    AWeaponBase* ExistingWeapon = GetWeaponBySlot(TargetSlot);
    if (ExistingWeapon) // if (!ExistingWeapon) return; 을 삭제했습니다.
    {
        if (ActiveWeapon == ExistingWeapon && WeaponAS)
        {
            ExistingWeapon->CurrentAmmoCount = FMath::RoundToInt(WeaponAS->GetCurrentAmmo());
        }
        ExistingWeapon->DetachFromCharacter();
        ExistingWeapon->Destroy();
    }

    // --- 2. 신규 무기 생성 ---
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerCharacter;
    SpawnParams.Instigator = OwnerCharacter; // 총알 발사 시 주인을 알기 위해 추가

    // AWeaponBase 클래스를 스폰 (메시는 나중에 InitializeWeapon에서 로드됨)
    AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(AWeaponBase::StaticClass(), FTransform::Identity, SpawnParams);
    if (!NewWeapon) return;

    if (TargetSlot == EWeaponSlot::Primary) PrimaryWeapon = NewWeapon;
    else SecondaryWeapon = NewWeapon;

    // --- 3. 상태 결정 (Active/Inactive) ---
    // 첫 무기이거나, 현재 들고 있는 슬롯과 같은 슬롯에 장착할 때 Active가 됨
    bool bShouldBecomeActive = (ActiveWeapon == nullptr || CurrentSlot == TargetSlot);

    if (bShouldBecomeActive)
    {
        // 기존 활성 무기가 있었다면 저장하고 숨김
        if (ActiveWeapon && WeaponAS) 
        {
            ActiveWeapon->CurrentAmmoCount = FMath::RoundToInt(WeaponAS->GetCurrentAmmo());
            ActiveWeapon->DetachFromCharacter();
            ActiveWeapon->SetActorHiddenInGame(true);
            ActiveWeapon->bIsActiveWeapon = false;
        }

        ActiveWeapon = NewWeapon;
        CurrentSlot = TargetSlot;
        NewWeapon->bIsActiveWeapon = true;
        
        OnActiveWeaponTagChanged.Broadcast(Tag);
    }
    else
    {
        NewWeapon->bIsActiveWeapon = false;
        NewWeapon->SetActorHiddenInGame(true);
    }

    // --- 4. 무기 초기화 및 부착 ---
    NewWeapon->InitializeWeapon(Tag, OwnerCharacter);
    NewWeapon->InitializeAttributes();
}

/** * [핵심 로직 2] 무기 교체 (Swap)
 * 현재 무기의 상태를 저장하고 타겟 무기의 상태를 불러옵니다.
 */
void UCombatComponent::SwapToSlot(EWeaponSlot TargetSlot)
{
    // 동일한 슬롯으로의 교체는 무시합니다.
    if (CurrentSlot == TargetSlot) return;

    AWeaponBase* TargetWeapon = GetWeaponBySlot(TargetSlot);
    if (!TargetWeapon) return; // 타겟 슬롯이 비어있으면 교체 불가

    UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(GetOwnerASC()->GetSet<UWeaponAttributeSet>());
    if (!WeaponAS) return;

    // 1. [SAVE] 현재 활성 무기의 실시간 탄약 정보를 액터로 백업합니다.
    if (!ActiveWeapon) return;
    ActiveWeapon->CurrentAmmoCount = FMath::RoundToInt(WeaponAS->GetCurrentAmmo());
    ActiveWeapon->DetachFromCharacter();
    ActiveWeapon->SetActorHiddenInGame(true);

    // 2. [SWITCH] 슬롯 및 포인터 전환
    ActiveWeapon = TargetWeapon;
    CurrentSlot = TargetSlot;

    // 3. [LOAD] 새 무기를 시각화하고 저장된 탄약 데이터를 캐릭터 스탯(GAS)에 주입합니다.
    ActiveWeapon->bIsActiveWeapon = true;
    ActiveWeapon->SetActorHiddenInGame(false);
    ActiveWeapon->AttachToCharacter();

    const FWeaponData& Data = ActiveWeapon->GetCurrentDataAsset()->WeaponData;
    // 0/0 표시 문제를 방지하기 위해 MaxAmmo부터 설정 후 CurrentAmmo를 로드합니다.
    WeaponAS->SetMaxAmmo(Data.DefaultMaxAmmo);
    WeaponAS->SetCurrentAmmo(static_cast<float>(ActiveWeapon->CurrentAmmoCount));
    
    OnActiveWeaponTagChanged.Broadcast(Data.WeaponTag);
}

/** * [유틸리티] 슬롯 토글 (단일 키 대응)
 */
void UCombatComponent::ToggleWeaponSwap()
{
    EWeaponSlot TargetSlot = (CurrentSlot == EWeaponSlot::Primary) ? EWeaponSlot::Secondary : EWeaponSlot::Primary;
    SwapToSlot(TargetSlot);
}

/** * [유틸리티] 소유자 ASC 조회 
 */
UAbilitySystemComponent* UCombatComponent::GetOwnerASC() const
{
    return OwnerCharacter ? OwnerCharacter->GetAbilitySystemComponent() : nullptr;
}

void UCombatComponent::PerformTrace()
{
    // 사격 판정 로직 구현 공간
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastFireTime > RecoilRecoveryDelay)
    {
        TargetRecoil = FMath::Vector2DInterpTo(TargetRecoil, FVector2D::ZeroVector, DeltaTime, RecoveryInterpSpeed);
    }
    FVector2D NewRecoil = FMath::Vector2DInterpTo(CurrentRecoil, TargetRecoil, DeltaTime, RecoilInterpSpeed);
    FVector2D RecoilDelta = NewRecoil - CurrentRecoil;
    CurrentRecoil = NewRecoil;
    
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;
    APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
    if (!PlayerController) return;
    
    PlayerController->AddPitchInput(RecoilDelta.X);
    PlayerController->AddYawInput(RecoilDelta.Y);
}

void UCombatComponent::ApplyRecoil()
{
    if (!ActiveWeapon) return;
    
    const FWeaponData& WeaponData = ActiveWeapon->GetCurrentDataAsset()->WeaponData;
    float RecoilAmount = WeaponData.DefaultRecoilValue;

    TargetRecoil.X += -RecoilAmount;

    float RandomYaw = FMath::RandRange(-RecoilAmount * 0.2f, RecoilAmount * 0.2f);
    TargetRecoil.Y += RandomYaw;
    
    LastFireTime = GetWorld()->GetTimeSeconds();
}
