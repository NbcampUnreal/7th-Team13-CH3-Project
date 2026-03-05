#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Zoom.generated.h"

UCLASS()
class FINALMINUTES_API UGA_Zoom : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Zoom();
	
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zoom")
	TSubclassOf<UGameplayEffect> ZoomEffectClass;
	
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	
	FActiveGameplayEffectHandle ActiveZoomEffectHandle;
};