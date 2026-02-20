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
	//바꿔야함 브금은 2~3개니까 시간에 따라 바뀌니까

private:
	UPROPERTY()
	TObjectPtr<class UDataTable> SoundDataTable;
};