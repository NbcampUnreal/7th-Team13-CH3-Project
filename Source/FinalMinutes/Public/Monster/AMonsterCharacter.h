#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AMonsterCharacter.generated.h"

class UAbilitySystemComponent;
class UAbilitySystemInterface;

UCLASS()
class FINALMINUTES_API AAMonsterCharacter : public APawn
{
	GENERATED_BODY()

public:
	AAMonsterCharacter();

	virtual void BeginPlay() override;
	
protected:
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GAS")
	TObjectPtr<class UAbilitySystemComponent> ASC;
	
private:
	
	bool bIsDead;
	float MonsterHP;
	float MonsterATK;
	float MonsterSpeed;
	float HeadDef;
	float BodyDef;
};
