#include "AbilitySystem/Abilities/GA_Prone.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsModule.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_Prone::UGA_Prone()
{
	// 어빌리티가 실행될때 액터당 하나의 인스턴스(객체)만 생성해서 재사용하겠다.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Ability Tags, 어빌리티 자체에 붙여주는 태그
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Prone")));
	
	// 이태그가 있으면 실행안함 
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
	UE_LOG(LogTemp, Warning, TEXT("UGA_Prone::ActivateAbility()"));
	// 스킬 사용을 위한 조건 체크 , 쿨타임, 코스트다 있는지
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		// Commit 실패 (스태미너 부족, Cooldown 중 등)
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 실제 Montage 재생,  이 애니메이션이 끝났는지/도중에 취소되었는지/중단되었는지를 감시하는 프록시(대리인)객체 생성
	UAbilityTask_PlayMontageAndWait* PlayMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, // 어빌리티 자기 자신
			NAME_None, // Task 별명
			ProneMontage, // 실제 재생할 Montage
			1.0f // 재생 속도
		);

	if (PlayMontageTask)
	{
		// Montage 완료 콜백 연결
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_Prone::OnMontageCompleted);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_Prone::OnMontageCancelled);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_Prone::OnMontageCancelled);
		// Task 활성화
		PlayMontageTask->ReadyForActivation();
	}

	// Gameplay Event 대기
	UAbilityTask_WaitGameplayEvent* WaitEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			FGameplayTag::RequestGameplayTag(FName("Event.Montage.Prone"))
		);

	if (WaitEventTask)
	{
		WaitEventTask->EventReceived.AddDynamic(this, &UGA_Prone::OnProneGameplayEvent);
		WaitEventTask->ReadyForActivation();
	}
	
	// 입력 해제 대기
	UAbilityTask_WaitGameplayEvent* WaitProneEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		FGameplayTag::RequestGameplayTag(FName("State.Prone.End"))
	);
	if (WaitProneEventTask)
	{
		// 이벤트 델리게이트, 버튼을 떼면 OnInputReleased를 실행해달라
		WaitProneEventTask->EventReceived.AddDynamic(this, &UGA_Prone::OnInputReleased);
		// Task 활성화 (감시자 작동)
		WaitProneEventTask->ReadyForActivation();
	}
}


void UGA_Prone::OnProneGameplayEvent(FGameplayEventData EventData)
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();

	if (!Character || !MyASC) return;
	
	// 컨텍스트 생성, 효과가 어디서부터 나타났는지, 추후에 데미지 계산이나 로그 시스템에서 누가 사용한지 알 수 있음
	FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	EffectContext.AddSourceObject(Character);

	// 실제로 적용될 효과를 작성, GE클래스
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
		ProneEffectClass,
		1.0f, // Level
		EffectContext
	);
	
	if (SpecHandle.IsValid())
	{
		// 나에게 발생하므로 나한테 효과 적용
		ActiveProneEffectHandle = MyASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UGA_Prone::OnInputReleased(FGameplayEventData EventData)
{
	// 버튼을 떼면 종료
	UE_LOG(LogTemp, Warning, TEXT("Input Released!"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Prone::OnMontageCompleted()
{
	// Ability 정상 종료
	// EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Prone::OnMontageCancelled()
{
	// Ability 취소 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
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