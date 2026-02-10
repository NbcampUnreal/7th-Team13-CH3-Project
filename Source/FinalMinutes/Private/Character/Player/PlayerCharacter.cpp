#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "AbilitySystem/Attributes/SensorAttributeSet.h"

APlayerCharacter::APlayerCharacter()
{
	// ASC 달아주기
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	// 사용할 AttributeSet 설정
	CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	SensorAttributeSet = CreateDefaultSubobject<USensorAttributeSet>(TEXT("SensorAttributeSet"));
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// ASC초기화
	InitializeAbilitySystem();
}

void APlayerCharacter::InitializeAbilitySystem()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

