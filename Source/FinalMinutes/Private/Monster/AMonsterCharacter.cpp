#include "Monster/AMonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "Attribute Set/MonsterAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"

AAMonsterCharacter::AAMonsterCharacter()
{
	// ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
}

UAbilitySystemComponent* AAMonsterCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}
