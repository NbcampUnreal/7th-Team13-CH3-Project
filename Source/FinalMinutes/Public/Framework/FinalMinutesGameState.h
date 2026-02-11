#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FinalMinutesGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillCountChanged, int32, NewKillCount);

UCLASS()
class FINALMINUTES_API AFinalMinutesGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	void AddKill();
	

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnKillCountChanged OnKillCountChanged;

private:
	int32 KillCount = 0;
};