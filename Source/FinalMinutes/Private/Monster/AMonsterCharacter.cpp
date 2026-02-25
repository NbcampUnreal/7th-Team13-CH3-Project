#include "Monster/AMonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"

AAMonsterCharacter::AAMonsterCharacter()
{
	// ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("MonsterAttributeSet"));
  
	// Capsule
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->InitCapsuleSize(35.f, 90.f);
	CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = CapsuleComp;
	
	// Move
	MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MonsterMovement"));
	
	MovementComp->MaxSpeed = 150.f;
	MovementComp->Acceleration = 100.f;
}

UAbilitySystemComponent* AAMonsterCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}
