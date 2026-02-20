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

void UCombatComponent::Fire()
{
	// ASC를 통해 입력 태그를 전달하여 GA_Fire 실행
    if (UAbilitySystemComponent* ASC = GetOwnerASC())
    {
        // 1. 실행하고자 하는 어빌리티가 가진 태그를 컨테이너에 담습니다.
        FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(TEXT("Weapon.State.Fire"));
        FGameplayTagContainer TagContainer(FireTag);

        // 2. 해당 태그를 가진 어빌리티를 찾아 실행을 시도합니다.
        ASC->TryActivateAbilitiesByTag(TagContainer);
    }
}

UAbilitySystemComponent* UCombatComponent::GetOwnerASC() const
{
    // OwnerCharacter가 불완전한 타입이 아니어야 GetAbilitySystemComponent() 호출 가능
    return OwnerCharacter ? OwnerCharacter->GetAbilitySystemComponent() : nullptr;
}

void UCombatComponent::Reload()
{
}

void UCombatComponent::EquipWeapon(FGameplayTag Tag)
{
	if (!OwnerCharacter || !Tag.IsValid()) return;

    // 1. 전용 서브시스템을 통한 무기 데이터 조회
    UWeaponRegistrySubsystem* Registry = GetWorld()->GetGameInstance()->GetSubsystem<UWeaponRegistrySubsystem>();
    if (!Registry) return;

    UWeaponDataAsset* WeaponData = Registry->GetWeaponDataByTag(Tag);
    if (!WeaponData)
    {
        UE_LOG(LogTemp, Error, TEXT("CombatComp: Failed to find WeaponData for Tag: %s"), *Tag.ToString());
        return;
    }

    // 2. 기존 무기 및 어빌리티 정리
    if (CurrentWeapon)
    {
        // 무기 해제 시 해당 무기가 부여했던 어빌리티나 이펙트를 제거하는 로직이 여기에 포함되어야 함
        CurrentWeapon->Destroy();
        CurrentWeapon = nullptr;
    }

    // 3. 무기 액터 스폰 및 초기화
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerCharacter;
    SpawnParams.Instigator = OwnerCharacter;

    // AWeaponBase를 기반으로 월드에 물리적으로 생성
    CurrentWeapon = GetWorld()->SpawnActor<AWeaponBase>(AWeaponBase::StaticClass(), FTransform::Identity, SpawnParams);
    
    if (CurrentWeapon)
    {
        // InitializeWeapon 내부에서 비동기 로딩 및 부착이 수행됨
        CurrentWeapon->InitializeWeapon(Tag, OwnerCharacter);
    }
    
    if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
    {
        // 데이터 에셋에 등록된 사격 어빌리티 클래스를 가져옵니다.
        if (TSubclassOf<UGameplayAbility> FireAbilityClass = WeaponData->WeaponData.FireAbilityClass)
        {
            ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetMaxAmmoAttribute(), WeaponData->WeaponData.DefaultMaxAmmo);
            ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetCurrentAmmoAttribute(), WeaponData->WeaponData.DefaultMaxAmmo);
            
            FGameplayAbilitySpec FireSpec(FireAbilityClass);
            // 입력 태그 매핑
            FireSpec.DynamicAbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Weapon.State.Fire")));
            
            FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(FireSpec);
        }
    }
}

void UCombatComponent::SwapWeapon()
{
}

void UCombatComponent::ToggleZoom()
{
}

void UCombatComponent::PerformTrace()
{
}

