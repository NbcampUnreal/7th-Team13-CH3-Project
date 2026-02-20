#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Reload.generated.h"

UCLASS()
class FINALMINUTES_API UGA_Reload : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Reload();
	
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
	// 장전 Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload")
	UAnimMontage* ReloadMontage;

	// 움직이며 장전 Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload")
	UAnimMontage* ReloadMoveMontage;
	
	// 앉아서 장전 Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload")
	UAnimMontage* ReloadCrouchMontage;
	
	// 엎드려서 장전 Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload")
	UAnimMontage* ReloadProneMontage;
	
	// 적용할 Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload")
	TSubclassOf<UGameplayEffect> ReloadEffectClass;
	
	// Montage 완료 콜백
	UFUNCTION()
	void OnMontageEnded();
	
	// 적용된 이펙트를 삭제하기 위한 핸들 변수
	FActiveGameplayEffectHandle ActiveReloadEffectHandle;
	
	// Gameplay Event 콜백 (AnimNotify 대신)
	UFUNCTION()
	void OnReloadGameplayEvent(FGameplayEventData EventData);
};
