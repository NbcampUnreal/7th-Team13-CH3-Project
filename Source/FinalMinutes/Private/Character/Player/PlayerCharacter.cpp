#include "Character/Player/PlayerCharacter.h"

APlayerCharacter::APlayerCharacter()
{
	// 초기화 작업
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

