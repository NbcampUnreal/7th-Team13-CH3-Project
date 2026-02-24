#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "AMonsterCharacter.generated.h"

class UAbilitySystemComponent;
class UAbilitySystemInterface;
class UMonsterAttributeSet;
class UFloatingPawnMovement;

UCLASS()
class FINALMINUTES_API AAMonsterCharacter : public APawn
{
	GENERATED_BODY()

public:
	AAMonsterCharacter();
	
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const;
	
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
	TObjectPtr<class UMonsterAttributeSet> AttributeSet;
	
	UPROPERTY()
	class UAbilitySystemComponent* AbilitySystemComponent;
};
