#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "SoundSubsystem.generated.h"

UCLASS()
class FINALMINUTES_API USoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlaySoundTag(FGameplayTag SoundTag, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayBGM(FGameplayTag BGMTag);

private:
	UPROPERTY()
	TObjectPtr<class UDataTable> SoundDataTable;
};