#include "AbilitySystem/Abilities/GA_Crouch.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/CapsuleComponent.h"

UGA_Crouch::UGA_Crouch()
{
	// 어빌리티가 실행될때 액터당 하나의 인스턴스(객체)만 생성해서 재사용하겠다.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Ability Tags, 어빌리티 자체에 붙여주는 태그
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Crouch")));
	
	// 앉기 어빌리티가 켜질때 엎드리기상태였다면 엎드리기 어빌리티가 실행중이라면 취소
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Prone")));
	
	// 이태그가 있으면 실행안함 (앉아있을때 또 앉을 수 없음)
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsCrouching")));

	// 소유태그 / 실행중 어떤 태그를 가질지
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsCrouching")));
}

void UGA_Crouch::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	if (!MyASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		// Commit 실패 (스태미너 부족, Cooldown 중 등)
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	Character->GetCapsuleComponent()->SetCapsuleHalfHeight(60.f);
	Character->GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -48.f));
	FGameplayEffectContextHandle EffectContext = MyASC->MakeEffectContext();
	EffectContext.AddSourceObject(Character);

	FGameplayEffectSpecHandle SpecHandle = MyASC->MakeOutgoingSpec(CrouchEffectClass, 1.0f, EffectContext);
    
	if (SpecHandle.IsValid())
	{
		ActiveCrouchEffectHandle = MyASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	// 입력 해제 대기
	UAbilityTask_WaitGameplayEvent* WaitCrouchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		FGameplayTag::RequestGameplayTag(FName("State.Crouch.End"))
	);
	if (!WaitCrouchEventTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	// 이벤트 델리게이트, 앉기가 끝나면 OnCrouchExitRequested호출해달라
	WaitCrouchEventTask->EventReceived.AddDynamic(this, &UGA_Crouch::OnCrouchExitRequested);
	// Task 활성화 (감시자 작동)
	WaitCrouchEventTask->ReadyForActivation();
	
}

void UGA_Crouch::OnCrouchExitRequested(FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Crouch::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;
	Character->GetCapsuleComponent()->SetCapsuleHalfHeight(90.f); 
	Character->GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

	if (Character->GetMesh() && Character->GetMesh()->GetAnimInstance())
	{
		Character->GetMesh()->GetAnimInstance()->Montage_Stop(0.2f, CrouchMontage);
	}
	
	// 추가 정리작업
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	if (!MyASC) return;
	if (ActiveCrouchEffectHandle.IsValid())
	{
		bool bRemoved = MyASC->RemoveActiveGameplayEffect(ActiveCrouchEffectHandle);
		ActiveCrouchEffectHandle.Invalidate(); // 핸들 초기화
	}
	
	// 부모 클래스 호출 (Tag 제거 등)
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
