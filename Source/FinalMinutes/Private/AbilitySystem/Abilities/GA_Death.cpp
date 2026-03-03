#include "AbilitySystem/Abilities/GA_Death.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_Death::UGA_Death()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

}

void UGA_Death::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!DeathEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 추후에 누가 죽였는지 필요하면 작성하고 우선은 빼두자
	// FGameplayEffectContextHandle EffectContext = MakeEffectContext(Handle, ActorInfo);
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DeathEffectClass, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
	
	
	float StateValue = TriggerEventData->EventMagnitude;
	UAnimMontage* SelectedMontage = DeathMontage;
	
	if (StateValue == 1.0f)
	{
		SelectedMontage = CrouchDeathMontage;
	}
	else if (StateValue == 2.0f)
	{
		SelectedMontage = ProneDeathMontage;
	}

	UAbilityTask_PlayMontageAndWait* PlayMontageTask = 
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		SelectedMontage,
		1.0f
		);
	
	if (!PlayMontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_Death::OnMontageEnded);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_Death::OnMontageEnded);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_Death::OnMontageEnded);
	PlayMontageTask->ReadyForActivation();
}

void UGA_Death::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}