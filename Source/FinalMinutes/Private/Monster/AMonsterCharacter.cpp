#include "Monster/AMonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"

AAMonsterCharacter::AAMonsterCharacter()
{
	// ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("AttributeSet"));
	
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

void AAMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (ASC)
	{
		// 바인딩
		ASC->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* AAMonsterCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

void AAMonsterCharacter::OnHitReaction_Implementation(const FHitResult& Hit)
{
	
}

