#include "Monster/Zombie.h"


AZombie::AZombie()
{
	PrimaryActorTick.bCanEverTick = false;
	
}

void AZombie::BeginPlay()
{
	Super::BeginPlay();
	
}

void AZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


