// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Fire.generated.h"

/**
 * 
 */
UCLASS()
class FINALMINUTES_API UGA_Fire : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UGA_Fire();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle, 
        const FGameplayAbilityActorInfo* ActorInfo, 
        const FGameplayAbilityActivationInfo ActivationInfo, 
        const FGameplayEventData* TriggerEventData) override;

protected:
    /** 탄약 소모를 위한 효과 클래스 */
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> AmmoCostEffectClass;

    /** 투사체에 주입할 데미지 효과 클래스 */
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

private:
    /** 실제 투사체 생성 로직 */
    void SpawnProjectile();
};
