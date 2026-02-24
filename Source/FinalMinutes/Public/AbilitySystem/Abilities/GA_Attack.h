#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Attack.generated.h"

UCLASS()
class FINALMINUTES_API UGA_Attack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Attack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr
	) const override;

	// Ability 종료
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	// 연사모드인지 (추후에 무기컴포넌트에서 불러오는식으로 변경)
	UPROPERTY(EditAnywhere)
	bool bIsRepeat = true;

	void PlayAttack();
	// 공격키를 누르고 있는지 판단
	bool bIsInputPressed = true;

protected:
	// 공격 Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	UAnimMontage* StandAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	UAnimMontage* CrouchAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	UAnimMontage* ProneAttackMontage;

	// 적용할 Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TSubclassOf<UGameplayEffect> AttackEffectClass;

	// Montage 완료 콜백
	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	// 적용된 이펙트를 삭제하기 위한 핸들 변수
	FActiveGameplayEffectHandle ActiveAttackEffectHandle;

	// Gameplay Event 콜백 (AnimNotify 대신)
	UFUNCTION()
	void OnAttackGameplayEvent(FGameplayEventData EventData);

	/** 실제 투사체 생성 로직 */
	UFUNCTION()
	void SpawnProjectile();

	/** 투사체에 주입할 데미지 효과 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};
