#include "AbilitySystem/Abilities/GA_Prone.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/CapsuleComponent.h"

UGA_Prone::UGA_Prone()
{
	// 어빌리티가 실행될때 액터당 하나의 인스턴스(객체)만 생성해서 재사용하겠다.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Ability Tags, 어빌리티 자체에 붙여주는 태그
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Prone")));
	
	// 엎드리기가 켜질때 앉기상태였다면 앉기 어빌리티가 실행중이라면 취소
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Crouch")));
	// 이태그가 있으면 실행안함, 현재는 OnProne에서 State.Prone.End을 호출해서 종료하는식이라서 필요없어 보이지만
	// 이 Blcoked태그가 없으면 연타했을때 이상하게 작동함
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning")));

	// 소유태그 / 실행중 어떤 태그를 가질지
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning")));
}

void UGA_Prone::ActivateAbility(
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
	AActor* Avatar = GetAvatarActorFromActorInfo();
	ACharacter* Character = Cast<ACharacter>(Avatar);
	if (!MyASC || !ProneEffectClass) // 클래스 할당 여부 체크 필수
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (Character)
	{
		// 기존 높이 저장
		Character->GetCapsuleComponent()->SetCapsuleHalfHeight(30.f);
		Character->AddActorWorldOffset(FVector(0.f, 0.f, -60.f));

		Character->GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -30.f));
	}
	
	FGameplayEffectContextHandle EffectContext = MyASC->MakeEffectContext();
	EffectContext.AddSourceObject(Avatar);
	FGameplayEffectSpecHandle SpecHandle = MyASC->MakeOutgoingSpec(ProneEffectClass, 1.0f, EffectContext);
    
	if (SpecHandle.IsValid())
	{
		ActiveProneEffectHandle = MyASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	
	// 입력 해제 대기
	UAbilityTask_WaitGameplayEvent* WaitProneEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		FGameplayTag::RequestGameplayTag(FName("State.Prone.End"))
	);
	
	if (WaitProneEventTask)
	{
		// 이벤트 델리게이트
		WaitProneEventTask->EventReceived.AddDynamic(this, &UGA_Prone::OnProneExitRequested);
		// Task 활성화 (감시자 작동)
		WaitProneEventTask->ReadyForActivation();
	}
}

void UGA_Prone::OnProneExitRequested(FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Prone::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		
		if (Character->GetMesh() && Character->GetMesh()->GetAnimInstance())
		{
			Character->GetMesh()->GetAnimInstance()->Montage_Stop(0.2f, ProneMontage);
			Character->GetCapsuleComponent()->SetCapsuleSize(34.f, 90.f);
			Character->GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		}
	}
	
	// 추가 정리작업
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	if (MyASC && ActiveProneEffectHandle.IsValid())
	{
		bool bRemoved = MyASC->RemoveActiveGameplayEffect(ActiveProneEffectHandle);
		ActiveProneEffectHandle.Invalidate(); // 핸들 초기화
	}
	
	// 부모 클래스 호출 (Tag 제거 등)
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}