#include "FinalMinutes/Public/AMonsterCharacter.h"

AAMonsterCharacter::AAMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AAMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAMonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

