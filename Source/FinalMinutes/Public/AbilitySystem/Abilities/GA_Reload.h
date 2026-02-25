#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Reload.generated.h"

/**
 * UGA_Reload
 * 무기 재장전 로직을 담당하는 어빌리티입니다.
 * 애니메이션 몽타주와 게임플레이 이벤트를 연동하여 탄환을 보충합니다.
 */
UCLASS()
class FINALMINUTES_API UGA_Reload : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_Reload();

    // --- UGameplayAbility Overrides ---
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
    // --- Internal Logic ---

    /** 자세별 몽타주 결정 및 관련 Task(몽타주, 이벤트 대기)를 생성합니다. */
    void SetupReloadTasks(class UAbilitySystemComponent* ASC, class APlayerCharacter* PlayerCharacter);

    /** 애니메이션의 특정 시점에 호출되어 실제 탄환 수치를 변경합니다. */
    UFUNCTION()
    void OnReloadGameplayEvent(FGameplayEventData EventData);

    /** 몽타주 재생이 종료(완료/중단/취소)되었을 때 호출됩니다. */
    UFUNCTION()
    void OnMontageEnded();

protected:
    // --- Assets & Effects ---

    UPROPERTY(EditDefaultsOnly, Category = "Effects | Animation")
    TObjectPtr<UAnimMontage> ReloadMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Effects | Animation")
    TObjectPtr<UAnimMontage> ReloadMoveMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Effects | Animation")
    TObjectPtr<UAnimMontage> ReloadCrouchMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Effects | Animation")
    TObjectPtr<UAnimMontage> ReloadProneMontage;

    /** 탄환 보충 로직이 담긴 게임플레이 이펙트 */
    UPROPERTY(EditDefaultsOnly, Category = "Effects | GameplayEffect")
    TSubclassOf<class UGameplayEffect> ReloadEffectClass;

private:
    /** 적용 중인 재장전 이펙트의 핸들 (종료 시 제거용) */
    FActiveGameplayEffectHandle ActiveReloadEffectHandle;
};