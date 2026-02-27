#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/Damageable.h"
#include "Components/CapsuleComponent.h"
#include "AMonsterCharacter.generated.h"

struct FGameplayEffectModCallbackData;
class UAbilitySystemComponent;
class UAbilitySystemInterface;
class UCharacterAttributeSet;
class UFloatingPawnMovement;

UCLASS()
class FINALMINUTES_API AAMonsterCharacter : public APawn, public IAbilitySystemInterface, public IDamageable
{
	GENERATED_BODY()

public:
	AAMonsterCharacter();
	
	// 인터페이스 구현 명시
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	// IDamageable 구현 (총알이 호출함)
	virtual void OnHitReaction_Implementation(const FHitResult& Hit) override;
	
protected:
	
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GAS")
	TObjectPtr<class UAbilitySystemComponent> ASC;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Collision")
	TObjectPtr<UCapsuleComponent> CapsuleComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	TObjectPtr<UFloatingPawnMovement> MovementComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	FRotator RotationRate;
	
	UPROPERTY()
	TObjectPtr<class UCharacterAttributeSet> AttributeSet;
	
	UPROPERTY()
	class UAbilitySystemComponent* AbilitySystemComponent;
};
