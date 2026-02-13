#include "Monster/AMonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

AAMonsterCharacter::AAMonsterCharacter()
{
	// ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	
	//Rotation
	
	
	//Movement
	
	
}

void AAMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
}


UAbilitySystemComponent* AAMonsterCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}
