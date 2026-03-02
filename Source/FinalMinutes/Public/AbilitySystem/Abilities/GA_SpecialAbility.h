#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SpecialAbility.generated.h"

class UPostProcessComponent;
class UGameplayEffect;

UCLASS()
class FINALMINUTES_API UGA_SpecialAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_SpecialAbility();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		bool bReplicateEndAbility, bool bWasCancelled) override;
protected:
	/** 시간 계수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
	float SlowRate;
	
	/** 사람할테 적용될 배수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
	float SlowRateForPlayer;

	/** 적용할 Cue*/
	UPROPERTY(EditAnywhere, Category = "GAS | Cue")
	FGameplayTag SpecialAbilityCueTag;

	/** 지속적으로 게이지를 소모할 GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	TSubclassOf<UGameplayEffect> StaminaDrainGEClass;
	
	FActiveGameplayEffectHandle StaminaDrainHandle;
	
	/** 스태미너 변경시 호출*/
	void OnStaminaChanged(const FOnAttributeChangeData& Data);
	
	FDelegateHandle StaminaChangeDelegateHandle;
private:
	/** 시간 적용 함수*/
	void ApplyTime(float GlobalFactor, float PlayerFactor);
};
