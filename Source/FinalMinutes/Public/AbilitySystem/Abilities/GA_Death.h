#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Death.generated.h"

UCLASS()
class FINALMINUTES_API UGA_Death : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Death();
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	UAnimMontage* DeathMontage;
	
	UPROPERTY(EditAnywhere, Category = "Death")
	UAnimMontage* CrouchDeathMontage;

	UPROPERTY(EditAnywhere, Category = "Death")
	UAnimMontage* ProneDeathMontage;
	
	// 적용할 Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	TSubclassOf<UGameplayEffect> DeathEffectClass;
	
	UFUNCTION()
	void OnMontageEnded();
};
