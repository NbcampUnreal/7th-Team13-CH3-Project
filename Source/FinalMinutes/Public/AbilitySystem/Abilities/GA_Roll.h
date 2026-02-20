#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Roll.generated.h"

UCLASS()
class FINALMINUTES_API UGA_Roll : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Roll();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	// Ability 종료
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

protected:
	// 구르기 Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	UAnimMontage* RollMontage;

	// 적용할 Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	TSubclassOf<UGameplayEffect> RollEffectClass;
	
	// Montage 완료 콜백
	UFUNCTION()
	void OnMontageEnded();
	
	// 적용된 이펙트를 삭제하기 위한 핸들 변수
	FActiveGameplayEffectHandle ActiveRollEffectHandle;
	
	// Gameplay Event 콜백 (AnimNotify 대신)
	UFUNCTION()
	void OnRollGameplayEvent(FGameplayEventData EventData);
};
