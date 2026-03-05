#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SoundSubsystem.generated.h"

UCLASS()
class FINALMINUTES_API USoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	class UAudioComponent* BackgroundMusicComponent;
	
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayBGMByPhase(USoundBase* NewSound);
};