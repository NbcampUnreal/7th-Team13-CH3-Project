#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveSubsystem.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FINALMINUTES_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	//킰수, 생존시간, 슬롯 받아서 파일로 저장
	UFUNCTION(BlueprintCallable, Category = "SaveData")
	void SaveGameData(int32 CurrentKillCount, float SurviveTime, FString SlotName);
	
	UFUNCTION(BlueprintCallable, Category = "SaveData")
	void LoadGameData(FString SloatName);
	
	// 유저가 현재 플레이 중인 슬롯
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData")
	FString CurrentSlotName;
	
	UPROPERTY(BlueprintReadWrite, Category = "SaveSystem")
	bool bIsLoadingGame = false;
};
