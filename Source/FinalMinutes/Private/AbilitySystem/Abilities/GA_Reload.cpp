#include "AbilitySystem/Abilities/GA_Reload.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Character/Components/CombatComponent.h"
#include "Components/InventoryComponent.h"
#include "Items/Weapons/WeaponData.h"
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
    
    // 풀탄인 경우, 재장전하지 않고 종료
    const float CurrentAmmo = ASC->GetNumericAttribute(UWeaponAttributeSet::GetCurrentAmmoAttribute());
    const float MaxAmmo = ASC->GetNumericAttribute(UWeaponAttributeSet::GetMaxAmmoAttribute());
    if (FMath::IsNearlyEqual(CurrentAmmo, MaxAmmo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    
    // 4. 인벤토리 탄약 확인 (탄약이 없으면 재장전 불가)
    UInventoryComponent* Inventory = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
    FName AmmoItemID = CurrentWeapon->GetCurrentDataAsset()->WeaponData.AmmoItemID;

    if (!Inventory || Inventory->GetItemQuantity(AmmoItemID) <= 0)
    {
        // 탄약이 없으므로 어빌리티를 즉시 종료 (애니메이션 실행 안 함)
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 5. 재장전 애니메이션 및 이벤트 대기 설정
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
    APlayerCharacter* Player = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!ASC || !Player) return;

    UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>();
    UCombatComponent* Combat = Player->GetCombatComponent();
    AWeaponBase* CurrentWeapon = Combat ? Combat->GetActiveWeapon() : nullptr;

    // 1. 유효성 검사 (인벤토리와 무기 데이터가 있어야 함)
    if (!Inventory || !CurrentWeapon || !CurrentWeapon->GetCurrentDataAsset()) return;

    // 2. 현재 상태 파악
    const float CurrentAmmo = ASC->GetNumericAttribute(UWeaponAttributeSet::GetCurrentAmmoAttribute());
    const float MaxAmmo = ASC->GetNumericAttribute(UWeaponAttributeSet::GetMaxAmmoAttribute());
    
    // 무기 데이터 에셋에서 탄약 아이템 ID를 가져옴 (예: "Bullet_pistol")
    FName AmmoItemID = CurrentWeapon->GetCurrentDataAsset()->WeaponData.AmmoItemID; 
    
    // 인벤토리에서 현재 가지고 있는 수량 확인
    const int32 CarriedQuantity = Inventory->GetItemQuantity(AmmoItemID);

    // 3. 계산: (탄창에 부족한 양) vs (인벤토리에 실제 있는 양)
    int32 AmountNeeded = FMath::RoundToInt(MaxAmmo - CurrentAmmo);
    int32 ActualReloadAmount = FMath::Min(AmountNeeded, CarriedQuantity);

    if (ActualReloadAmount <= 0) return;

    // 4. [인벤토리 처리] 실제 아이템 수량 감소 (남은 총알 수 줄어듦)
    if (Inventory->ConsumeItem(AmmoItemID, ActualReloadAmount))
    {
        // 5. [GAS 처리] 인벤토리에서 성공적으로 뺀 만큼만 탄창(CurrentAmmo)을 채움
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddSourceObject(Player);

        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ReloadEffectClass, 1.0f, Context);
        if (SpecHandle.IsValid())
        {
            // GE_Reload의 Modifier Magnitude를 SetByCaller로 전달
            // 태그명은 GE_Reload 파일 내 Magnitude에 설정한 태그와 동일해야 함
            SpecHandle.Data.Get()->SetSetByCallerMagnitude(
                FGameplayTag::RequestGameplayTag(FName("Weapon.Data.ReloadAmount")), 
                (float)ActualReloadAmount
            );
            
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        }
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
