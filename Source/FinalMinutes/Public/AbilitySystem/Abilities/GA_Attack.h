#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Items/Weapons/FWeaponData.h"
#include "GA_Attack.generated.h"

UCLASS()
class FINALMINUTES_API UGA_Attack : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_Attack();
    bool CanActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                            const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                            FGameplayTagContainer* OptionalRelevantTags) const;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                            const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                            bool bWasCancelled) override;

protected:
    /** 실제 사격 논리 루프 (FireRate 기반) */
    UFUNCTION()
    void HandleFiringLoop();

    /** 발사 시 짧은 상체 반동 몽타주 재생 */
    void PlayRecoilMontage();

    /** 입력 해제 시 호출 */
    UFUNCTION()
    void OnInputReleased(float TimeHeld);

    /** 현재 무기 데이터 취득 헬퍼 */
    const struct FWeaponData* GetWeaponData() const;

    /** 실제 투사체 생성 */
    void SpawnProjectile() const;

protected:
    /** 수동 설정할 몽타주들 (반동용 짧은 Additive 권장) */
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TObjectPtr<UAnimMontage> StandRecoilMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TObjectPtr<UAnimMontage> CrouchRecoilMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TObjectPtr<UAnimMontage> ProneRecoilMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<class UGameplayEffect> AttackEffectClass;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<class UGameplayEffect> DamageEffectClass;

private:
    bool bIsInputPressed = false;
    FActiveGameplayEffectHandle ActiveAttackEffectHandle;
};
