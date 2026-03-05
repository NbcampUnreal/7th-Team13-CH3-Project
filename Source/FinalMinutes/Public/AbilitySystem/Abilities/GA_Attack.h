#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Attack.generated.h"

/**
 * UGA_Attack
 * 원거리 무기 발사 및 연사 로직을 담당하는 어빌리티입니다.
 */

class UCameraShakeBase;

UCLASS()
class FINALMINUTES_API UGA_Attack : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_Attack();

    // --- UGameplayAbility Overrides ---
    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle, 
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr,
        const FGameplayTagContainer* TargetTags = nullptr,
        FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle, 
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle, 
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, 
        bool bReplicateEndAbility,
        bool bWasCancelled) override;

protected:
    // --- Core Firing Logic ---

    /** 사격 루프 제어 (단발/연사 판정 및 재귀 호출) */
    UFUNCTION()
    void HandleFiringLoop();

    /** 실제 탄환(Projectile) 생성 및 발사 연출 실행 */
    void SpawnProjectile() const;

    /** 캐릭터의 자세에 따른 반동 애니메이션 재생 */
    void PlayRecoilMontage();
    
    /** 실제 반동 적용*/
    void ApplyRecoil();
    
    /** 사격 시 소음 발생 */
    void GenerateFiringNoise() const;

    // --- Helper Functions ---

    /** 현재 장착 중인 무기의 데이터 에셋 정보를 반환 */
    const struct FWeaponData* GetWeaponData() const;

    /** 화면 중앙(에임)을 기준으로 실제 탄환이 날아갈 목표 지점 계산 */
    FVector GetTargetLocation(class APlayerController* PC) const;

    /** 투사체에 전달할 데미지 이펙트 스펙 생성 */
    FGameplayEffectSpecHandle CreateDamageSpec(
        class APlayerCharacter* Instigator, 
        class AWeaponBase* Weapon,
        float Damage) const;

protected:
    // --- GAS & Animation Assets ---

    UPROPERTY(EditDefaultsOnly, Category = "Effects | Animation")
    TObjectPtr<UAnimMontage> StandRecoilMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Effects | Animation")
    TObjectPtr<UAnimMontage> CrouchRecoilMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Effects | Animation")
    TObjectPtr<UAnimMontage> ProneRecoilMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Effects | GameplayEffect")
    TSubclassOf<class UGameplayEffect> DamageEffectClass;

private:
    /** 연사 시 첫 번째 발사인지 확인하는 플래그 (코스트 중복 차감 방지) */
    bool bIsFirstShot = false;

    /** 적용 중인 공격 관련 이펙트 핸들 */
    FActiveGameplayEffectHandle ActiveAttackEffectHandle;
};
