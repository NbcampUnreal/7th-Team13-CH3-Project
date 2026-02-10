#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AMonsterCharacter.generated.h"

UCLASS()
class FINALMINUTES_API AAMonsterCharacter : public APawn
{
	GENERATED_BODY()

public:
	AAMonsterCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
