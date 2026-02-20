#pragma once

#include "CoreMinimal.h"
#include "Monster/AMonsterCharacter.h"
#include "Zombie.generated.h"

UCLASS()
class FINALMINUTES_API AZombie : public AAMonsterCharacter
{
	GENERATED_BODY()

public:
	AZombie();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
