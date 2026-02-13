#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Crouch.generated.h"

UCLASS()
class FINALMINUTES_API UGA_Crouch : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Crouch();
	
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
	// 앉기 Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crouch")
	UAnimMontage* CrouchMontage;

	// 적용할 Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crouch")
	TSubclassOf<UGameplayEffect> CrouchEffectClass;
	
	// Montage 완료 콜백
	UFUNCTION()
	void OnMontageCompleted();

	// Montage 취소 콜백
	UFUNCTION()
	void OnMontageCancelled();
	
	// 적용된 이펙트를 삭제하기 위한 핸들 변수
	FActiveGameplayEffectHandle ActiveCrouchEffectHandle;
	
	// 앉기 버튼을 뗄떼 호출
	UFUNCTION()
	void OnInputReleased(FGameplayEventData EventData);

	// Gameplay Event 콜백 (AnimNotify 대신)
	UFUNCTION()
	void OnCrouchGameplayEvent(FGameplayEventData EventData);
};
