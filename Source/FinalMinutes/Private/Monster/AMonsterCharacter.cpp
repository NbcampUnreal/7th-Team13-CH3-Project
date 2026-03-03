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
	MovementComp->MaxSpeed = 300.f;
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

float AAMonsterCharacter::GetDamageMultiplierForRegion(const FGameplayTagContainer& SpecAssetTags) const
{
	for (const auto& Pair : HitRegionMultipliers)
	{
		if (SpecAssetTags.HasTag(Pair.Key))
		{
			return Pair.Value;
		}
	}
	return 1.0f;
}

UAbilitySystemComponent* AAMonsterCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

void AAMonsterCharacter::OnHitReaction_Implementation(const FHitResult& Hit)
{
	
}

