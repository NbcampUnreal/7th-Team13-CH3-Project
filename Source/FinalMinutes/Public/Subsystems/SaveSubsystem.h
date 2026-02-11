#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveSubsystem.generated.h"

UCLASS()
class FINALMINUTES_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	//예시 아무거나 쓴거
	UFUNCTION(BlueprintCallable, Category = "Events")
	void SaveGame();
	
};
