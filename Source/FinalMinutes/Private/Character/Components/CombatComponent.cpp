// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/CombatComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Character/Player/PlayerCharacter.h"
#include "Items/Weapons/WeaponBase.h"
#include "Items/Weapons/WeaponDataAsset.h"
#include "Subsystems/WeaponRegistrySubsystem.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
    OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

UAbilitySystemComponent* UCombatComponent::GetOwnerASC() const
{
    // OwnerCharacter가 불완전한 타입이 아니어야 GetAbilitySystemComponent() 호출 가능
    return OwnerCharacter ? OwnerCharacter->GetAbilitySystemComponent() : nullptr;
}

class AWeaponBase* UCombatComponent::GetWeaponBySlot(EWeaponSlot Slot) const
{
    switch (Slot)
    {
    case EWeaponSlot::Primary:   return PrimaryWeapon;
    case EWeaponSlot::Secondary: return SecondaryWeapon;
    default:                     return nullptr;
    }
}

void UCombatComponent::EquipWeapon(FGameplayTag Tag)
{
    /*if (!OwnerCharacter || !Tag.IsValid()) return;

    // 장착할 무기의 상세 정보 불러오기
    UWeaponRegistrySubsystem* Registry = GetWorld()->GetGameInstance()->GetSubsystem<UWeaponRegistrySubsystem>();
    UWeaponDataAsset* WeaponDataAsset = Registry ? Registry->GetWeaponDataByTag(Tag) : nullptr;
    if (!WeaponDataAsset) return;

    // 무기 슬롯
    EWeaponSlot TargetSlot = WeaponDataAsset->WeaponData.DefaultSlot;


    // 1. [Save] 동일 슬롯에 이미 무기가 있다면 탄약 저장 후 교체 준비
    if (AWeaponBase* ExistingWeapon = GetWeaponBySlot(TargetSlot))
    {
        // 지워질 무기가 현재 들고 있던 무기라면 탄수 백업
        if (ActiveWeapon == ExistingWeapon)
        {
            if (UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(GetOwnerASC()->GetSet<UWeaponAttributeSet>()))
            {
                ExistingWeapon->SetCurrentAmmoCount(FMath::RoundToInt(WeaponAS->GetCurrentAmmo()));
            }
            ActiveWeapon = nullptr;
        }
        ExistingWeapon->DetachFromCharacter();
        /*ExistingWeapon->Destroy();#1#
    }
    
    // 1. [Save] 현재 들고 있는 무기가 있다면 탄약 저장 후 해제
    /*if (ActiveWeapon && CurrentSlot == TargetSlot)
    {
        if (UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(GetOwnerASC()->GetSet<UWeaponAttributeSet>()))
        {
            ActiveWeapon->SetCurrentAmmoCount(FMath::RoundToInt(WeaponAS->GetCurrentAmmo()));
        }
        ActiveWeapon->DetachFromCharacter();
    }#1#

    // 2. 신규 무기 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerCharacter;
    SpawnParams.Instigator = OwnerCharacter;
    AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(AWeaponBase::StaticClass(), FTransform::Identity, SpawnParams);
    if (!NewWeapon) return;

    // 3. 인벤토리 슬롯 할당
    if (TargetSlot == EWeaponSlot::Primary) PrimaryWeapon = NewWeapon;
    else if (TargetSlot == EWeaponSlot::Secondary) SecondaryWeapon = NewWeapon;

    // 4. 활성화 여부 결정
    bool bShouldBecomeActive = (ActiveWeapon == nullptr || CurrentSlot == TargetSlot);

    if (bShouldBecomeActive)
    {
        // 기존에 다른 슬롯 무기를 들고 있었다면 백업 후 숨김
        if (ActiveWeapon)
        {
            if (UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(GetOwnerASC()->GetSet<UWeaponAttributeSet>()))
            {
                ActiveWeapon->SetCurrentAmmoCount(FMath::RoundToInt(WeaponAS->GetCurrentAmmo()));
            }
            ActiveWeapon->DetachFromCharacter();
            ActiveWeapon->SetActorHiddenInGame(true);
        }

        ActiveWeapon = NewWeapon;
        CurrentSlot = TargetSlot;
        NewWeapon->bIsActiveWeapon = true;
    }
    else
    {
        NewWeapon->bIsActiveWeapon = false;
    }

    // 5. 무기 초기화 (이 안에서 활성 여부에 따라 AttributeSet을 건드릴지 결정함)
    NewWeapon->InitializeAttributes();
    NewWeapon->InitializeWeapon(Tag, OwnerCharacter);*/
    
    if (!OwnerCharacter || !Tag.IsValid()) return;

    UWeaponRegistrySubsystem* Registry = GetWorld()->GetGameInstance()->GetSubsystem<UWeaponRegistrySubsystem>();
    UWeaponDataAsset* WeaponDataAsset = Registry ? Registry->GetWeaponDataByTag(Tag) : nullptr;
    if (!WeaponDataAsset) return;

    EWeaponSlot TargetSlot = WeaponDataAsset->WeaponData.DefaultSlot;
    UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(GetOwnerASC()->GetSet<UWeaponAttributeSet>());

    // 1. 기존 슬롯 무기 제거 전 탄약 데이터 백업
    if (AWeaponBase* ExistingWeapon = GetWeaponBySlot(TargetSlot))
    {
        if (ActiveWeapon == ExistingWeapon && WeaponAS)
        {
            // 현재 쓰고 있던 무기라면 현재 ASC 값을 무기 액터에 저장
            ExistingWeapon->CurrentAmmoCount = FMath::RoundToInt(WeaponAS->GetCurrentAmmo());
            ActiveWeapon = nullptr;
        }
        ExistingWeapon->DetachFromCharacter();
        ExistingWeapon->Destroy();
    }

    // 2. 신규 무기 생성
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerCharacter;
    AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(AWeaponBase::StaticClass(), FTransform::Identity, SpawnParams);
    
    if (TargetSlot == EWeaponSlot::Primary) PrimaryWeapon = NewWeapon;
    else SecondaryWeapon = NewWeapon;

    // 3. 활성화 여부 결정
    bool bShouldBecomeActive = (ActiveWeapon == nullptr || CurrentSlot == TargetSlot);
    
    if (bShouldBecomeActive)
    {
        // 이미 다른 무기를 들고 있었다면 해당 무기 데이터 백업
        if (ActiveWeapon && WeaponAS)
        {
            ActiveWeapon->CurrentAmmoCount = FMath::RoundToInt(WeaponAS->GetCurrentAmmo());
            ActiveWeapon->DetachFromCharacter();
            ActiveWeapon->SetActorHiddenInGame(true);
        }
        
        ActiveWeapon = NewWeapon;
        CurrentSlot = TargetSlot;
        NewWeapon->bIsActiveWeapon = true;
    }
    else
    {
        NewWeapon->bIsActiveWeapon = false;
    }

    // 4. 무기 초기화 (InitializeAttributes 내부에서 Active 여부에 따라 0/0 방지 로직 실행)
    NewWeapon->InitializeWeapon(Tag, OwnerCharacter);
    NewWeapon->InitializeAttributes();
}

void UCombatComponent::PerformTrace()
{
}

void UCombatComponent::SwapToSlot(EWeaponSlot TargetSlot)
{
    /*if (CurrentSlot == TargetSlot) return;
    AWeaponBase* TargetWeapon = GetWeaponBySlot(TargetSlot);
    if (!TargetWeapon) return;

    UAbilitySystemComponent* ASC = GetOwnerASC();
    UWeaponAttributeSet* WeaponAS = ASC ? const_cast<UWeaponAttributeSet*>(ASC->GetSet<UWeaponAttributeSet>()) : nullptr;

    // 1. [Save] 기존 무기의 현재 탄약을 무기 액터에 저장
    if (ActiveWeapon && WeaponAS)
    {
        ActiveWeapon->SetCurrentAmmoCount(FMath::RoundToInt(WeaponAS->GetCurrentAmmo()));
        ActiveWeapon->DetachFromCharacter();
        ActiveWeapon->SetActorHiddenInGame(true);
    }

    // 2. 무기 교체
    ActiveWeapon = TargetWeapon;
    CurrentSlot = TargetSlot;

    // 3. [Load] 새 무기가 기억하던 탄약을 ASC Attribute에 주입
    if (ActiveWeapon && WeaponAS)
    {
        ActiveWeapon->bIsActiveWeapon = true;
        ActiveWeapon->SetActorHiddenInGame(false);
        ActiveWeapon->AttachToCharacter();

        // 저장되어 있던 탄수와 최대 탄수를 ASC에 즉시 반영
        const FWeaponData& Data = ActiveWeapon->GetCurrentDataAsset()->WeaponData;
        WeaponAS->SetMaxAmmo(Data.DefaultMaxAmmo);
        WeaponAS->SetCurrentAmmo(ActiveWeapon->CurrentAmmoCount);
    }*/
    
    if (CurrentSlot == TargetSlot) return;
    AWeaponBase* TargetWeapon = GetWeaponBySlot(TargetSlot);
    if (!TargetWeapon) return;

    UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(GetOwnerASC()->GetSet<UWeaponAttributeSet>());
    if (!WeaponAS) return;

    // 1. [SAVE] 기존 무기의 실시간 탄약 수치를 무기 액터로 저장
    if (ActiveWeapon)
    {
        ActiveWeapon->CurrentAmmoCount = FMath::RoundToInt(WeaponAS->GetCurrentAmmo());
        ActiveWeapon->DetachFromCharacter();
        ActiveWeapon->SetActorHiddenInGame(true);
    }

    // 2. 슬롯 교체
    ActiveWeapon = TargetWeapon;
    CurrentSlot = TargetSlot;

    // 3. [LOAD] 새 무기가 이전에 저장해둔 탄약 수치를 전역 AttributeSet에 주입
    ActiveWeapon->bIsActiveWeapon = true;
    ActiveWeapon->SetActorHiddenInGame(false);
    ActiveWeapon->AttachToCharacter();

    const FWeaponData& Data = ActiveWeapon->GetCurrentDataAsset()->WeaponData;
    // 이 시점에서 0/0 문제를 해결하기 위해 강제 주입합니다.
    WeaponAS->SetMaxAmmo(Data.DefaultMaxAmmo);
    WeaponAS->SetCurrentAmmo(static_cast<float>(ActiveWeapon->CurrentAmmoCount));
}

void UCombatComponent::ToggleWeaponSwap()
{
    EWeaponSlot TargetSlot = (CurrentSlot == EWeaponSlot::Primary) ? EWeaponSlot::Secondary : EWeaponSlot::Primary;
    SwapToSlot(TargetSlot);
}

