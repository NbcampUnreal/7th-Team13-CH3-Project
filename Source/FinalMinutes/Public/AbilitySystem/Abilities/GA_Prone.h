#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Prone.generated.h"

UCLASS()
class FINALMINUTES_API UGA_Prone : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Prone();
	
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
	// 엎드리기 Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prone")
	UAnimMontage* ProneMontage;

	// 적용할 Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prone")
	TSubclassOf<UGameplayEffect> ProneEffectClass;

	// 적용된 이펙트를 삭제하기 위한 핸들 변수
	FActiveGameplayEffectHandle ActiveProneEffectHandle;
	
	// 토글로 일어날때
	UFUNCTION()
	void OnProneExitRequested(FGameplayEventData EventData);

	// Gameplay Event 콜백 (AnimNotify 대신)
	UFUNCTION()
	void OnProneGameplayEvent(FGameplayEventData EventData);
};
