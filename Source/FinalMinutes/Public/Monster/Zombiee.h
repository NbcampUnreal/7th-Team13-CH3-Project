#pragma once

#include "CoreMinimal.h"
#include "Monster/AMonsterCharacter.h"
#include "Zombiee.generated.h"

UCLASS()
class FINALMINUTES_API AZombiee : public AAMonsterCharacter
{
	GENERATED_BODY()

public:
	AZombiee();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
