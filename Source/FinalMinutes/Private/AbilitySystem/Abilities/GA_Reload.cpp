#include "AbilitySystem/Abilities/GA_Reload.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Components/CombatComponent.h"
#include "Items/Weapons/FWeaponData.h"
#include "Items/Weapons/WeaponBase.h"
#include "Items/Weapons/WeaponDataAsset.h"

UGA_Reload::UGA_Reload()
{
    // 어빌리티 인스턴싱 정책: 액터당 하나 생성
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

}

void UGA_Reload::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // 1. 실행 조건 체크 (코스트/쿨다운)
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 2. 필요 컴포넌트 및 데이터 유효성 검사
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!ASC || !PlayerCharacter) return;

    UCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent();
    AWeaponBase* CurrentWeapon = CombatComponent ? CombatComponent->GetActiveWeapon() : nullptr;
    if (!CurrentWeapon || !CurrentWeapon->GetCurrentDataAsset())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 3. 재장전 애니메이션 및 이벤트 대기 설정
    SetupReloadTasks(ASC, PlayerCharacter);

    // 재장전 관련 시각/청각 효과 실행
    CurrentWeapon->ExecuteWeaponEffects(EWeaponActionType::Reload);
}

void UGA_Reload::SetupReloadTasks(UAbilitySystemComponent* ASC, APlayerCharacter* PlayerCharacter)
{
    // 1. 자세에 따른 몽타주 선택
    UAnimMontage* SelectedMontage = ReloadMontage;
    if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning"))))
    {
        SelectedMontage = ReloadProneMontage;
    }
    else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsCrouching"))))
    {
        SelectedMontage = ReloadCrouchMontage;
    }

    if (!SelectedMontage)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    // 2. 재장전 완료 이벤트 대기 Task (애니메이션 특정 시점에 탄환 장전)
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, FGameplayTag::RequestGameplayTag(FName("Event.Montage.Reload")));
    if (WaitEventTask)
    {
        WaitEventTask->EventReceived.AddDynamic(this, &UGA_Reload::OnReloadGameplayEvent);
        WaitEventTask->ReadyForActivation();
    }

    // 3. 몽타주 재생 Task
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, SelectedMontage);
    if (MontageTask)
    {
        MontageTask->OnBlendOut.AddDynamic(this, &UGA_Reload::OnMontageEnded);
        MontageTask->OnInterrupted.AddDynamic(this, &UGA_Reload::OnMontageEnded);
        MontageTask->OnCancelled.AddDynamic(this, &UGA_Reload::OnMontageEnded);
        MontageTask->ReadyForActivation();
    }
}

void UGA_Reload::OnReloadGameplayEvent(FGameplayEventData EventData)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC) return;

    // 장전 효과(GE) 적용 (탄창 수치 변경)
    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ReloadEffectClass, 1.0f, EffectContext);
    if (SpecHandle.IsValid())
    {
        ActiveReloadEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}


void UGA_Reload::OnMontageEnded()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Reload::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // 적용 중인 재장전 관련 GE 제거
    if (ActiveReloadEffectHandle.IsValid())
    {
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
        {
            ASC->RemoveActiveGameplayEffect(ActiveReloadEffectHandle);
        }
        ActiveReloadEffectHandle.Invalidate();
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
