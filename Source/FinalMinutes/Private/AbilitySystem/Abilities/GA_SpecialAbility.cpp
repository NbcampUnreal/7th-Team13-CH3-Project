#include "AbilitySystem/Abilities/GA_SpecialAbility.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"

UGA_SpecialAbility::UGA_SpecialAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    SlowRate = 0.2f;
    SlowRateForPlayer = 2.0f;
}

void UGA_SpecialAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    ApplyTime(SlowRate, SlowRateForPlayer);

    if (StaminaDrainGEClass)
    {
        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(StaminaDrainGEClass, 1.0f);
        if (SpecHandle.IsValid())
        {
            StaminaDrainHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
            if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
            {
                StaminaChangeDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetStaminaAttribute()).AddUObject(this, &UGA_SpecialAbility::OnStaminaChanged);
            }
        }
    }
}

void UGA_SpecialAbility::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
    if (Data.NewValue <= 0.0f)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UGA_SpecialAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, 
    bool bReplicateEndAbility, 
    bool bWasCancelled)
{
    if (!IsEndAbilityValid(Handle, ActorInfo))
    {
        return;
    }
    /** 기본 시간으로 돌림 */
    ApplyTime(1.0f, 1.0f);
    
    if (StaminaDrainHandle.IsValid())
    {
        GetAbilitySystemComponentFromActorInfo()->RemoveActiveGameplayEffect(StaminaDrainHandle);
    }
    
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetStaminaAttribute()).Remove(StaminaChangeDelegateHandle);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SpecialAbility::ApplyTime(float GlobalFactor, float PlayerFactor)
{
    if (UWorld* World = GetWorld())
    {
        World->GetWorldSettings()->SetTimeDilation(GlobalFactor);
    }

    if (AActor* Avatar = GetAvatarActorFromActorInfo())
    {
        Avatar->CustomTimeDilation = PlayerFactor;
    }
}

