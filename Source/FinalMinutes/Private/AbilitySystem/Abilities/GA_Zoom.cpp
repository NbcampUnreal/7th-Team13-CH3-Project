#include "AbilitySystem/Abilities/GA_Zoom.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Character/Player/PlayerCharacter.h"

UGA_Zoom::UGA_Zoom()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Zoom::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
	if (APlayerCharacter* Char = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (Char->IsInventoryOpen())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}
    // 스킬 사용 조건 체크 (코스트, 쿨타임 등)
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
       EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
       return;
    }

    UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	if (!MyASC || !ZoomEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	// Effect Context 세팅 및 Spec 생성
	FGameplayEffectContextHandle EffectContext = MyASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle SpecHandle = MyASC->MakeOutgoingSpec(ZoomEffectClass, 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
	  ActiveZoomEffectHandle = MyASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	
    // 줌은 버튼을 떼면 종료되므로 WaitInputRelease 사용
    UAbilityTask_WaitInputRelease* WaitReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);

    if (!WaitReleaseTask)
    {
    	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	    return;
    }
	// 신호가 오면 어빌리티 종료 함수 연결
	WaitReleaseTask->OnRelease.AddDynamic(this, &UGA_Zoom::OnInputReleased);
	WaitReleaseTask->ReadyForActivation();
    
}

void UGA_Zoom::OnInputReleased(float TimeHeld)
{
    // 입력이 끝나면 어빌리티 종료
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Zoom::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // 어빌리티 종료 시 GE(줌 상태 관련 효과 등) 제거
    if (ActiveZoomEffectHandle.IsValid())
    {
       GetAbilitySystemComponentFromActorInfo()->RemoveActiveGameplayEffect(ActiveZoomEffectHandle);
       ActiveZoomEffectHandle.Invalidate();
    }
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}