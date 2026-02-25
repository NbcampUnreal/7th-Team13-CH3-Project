#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AMonsterCharacter.generated.h"

class UAbilitySystemComponent;
class UAbilitySystemInterface;
class UCharacterAttributeSet;

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
	
	UPROPERTY()
	TObjectPtr<UCharacterAttributeSet> AttributeSet;
	
	UPROPERTY()
	class UAbilitySystemComponent* AbilitySystemComponent;
};
