#include "AbilitySystem/Abilities/GA_Sprint.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_Sprint::UGA_Sprint()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Sprint")));
	
	// 이태그가 있으면 실행안함 
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsSprinting")));

	// 소유태그 / 실행중 어떤 태그를 가질지
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsSprinting")));
}

void UGA_Sprint::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 스킬 사용을 위한 조건 체크 , 쿨타임, 코스트다 있는지
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		// Commit 실패 (스태미너 부족, Cooldown 중 등)
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	if (!MyASC) return;
	if (!SprintEffectClass) return;
	if (!SprintStaminaEffectClass) return;
	
	// 기존 이펙트에 스태미너 깎는 로직 추가
	FGameplayEffectContextHandle EffectContext = MyASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle SpecHandle = MyASC->MakeOutgoingSpec(SprintEffectClass, 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
		ActiveSprintEffectHandle = MyASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	
	FGameplayEffectSpecHandle StaminaSpecHandle = MyASC->MakeOutgoingSpec(SprintStaminaEffectClass, 1.0f, EffectContext);
	if (StaminaSpecHandle.IsValid())
	{
		FGameplayTag CostTag = FGameplayTag::RequestGameplayTag(FName("Player.Stats.Stamina"));
		StaminaSpecHandle.Data.Get()->SetSetByCallerMagnitude(CostTag, -1.0f);
		
		ActiveSprintStaminaEffectHandle = MyASC->ApplyGameplayEffectSpecToSelf(*StaminaSpecHandle.Data.Get());
	}

	// 스태미너 변화를 감지, 변화가 있을때마다 OnStaminaDepelted 호출
	StaminaChangeDelegateHandle = MyASC->GetGameplayAttributeValueChangeDelegate(
		   UCharacterAttributeSet::GetStaminaAttribute()
	   ).AddUObject(this, &UGA_Sprint::OnStaminaDepleted);

	UAbilityTask_WaitGameplayEvent* WaitStopTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		FGameplayTag::RequestGameplayTag(FName("Event.Montage.Sprint"))
	);

	if (!WaitStopTask) return;
	// 신호가 오면 어빌리티 종료 함수 연결
	WaitStopTask->EventReceived.AddDynamic(this, &UGA_Sprint::OnSprintStopReceived);
	WaitStopTask->ReadyForActivation();
}

// 스태미너가 깎일때마다 호출
void UGA_Sprint::OnStaminaDepleted(const FOnAttributeChangeData& Data)
{
	// 스태미나가 0 이하로 떨어졌다면 달리기를 강제 종료
	if (Data.NewValue <= 0.0f)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_Sprint::OnSprintStopReceived(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Sprint::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;
	
	// 어빌리티 종료시 GE(속도 증가 효과)제거
	if (ActiveSprintEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ActiveSprintEffectHandle);
		ActiveSprintEffectHandle.Invalidate();
	}

	if (ActiveSprintStaminaEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ActiveSprintStaminaEffectHandle);
		ActiveSprintStaminaEffectHandle.Invalidate();
	}

	if (StaminaChangeDelegateHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetStaminaAttribute()).Remove(StaminaChangeDelegateHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}