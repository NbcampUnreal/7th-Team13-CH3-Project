#include "Monster/Zombiee.h"


AZombiee::AZombiee()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AZombiee::BeginPlay()
{
	Super::BeginPlay();
	
}

void AZombiee::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


