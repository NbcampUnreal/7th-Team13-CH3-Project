#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Sprint.generated.h"

UCLASS()
class FINALMINUTES_API UGA_Sprint : public UGameplayAbility
{
	GENERATED_BODY()
	UGA_Sprint();
	
	virtual void ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;
protected:
	// Sprint Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	UAnimMontage* SprintMontage;

	// 적용할 Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	TSubclassOf<UGameplayEffect> SprintEffectClass;
	
	UFUNCTION()
	void OnSprintStopReceived(FGameplayEventData Payload);
	
	// 적용된 이펙트를 삭제하기 위한 핸들 변수
	FActiveGameplayEffectHandle ActiveSprintEffectHandle;
};
