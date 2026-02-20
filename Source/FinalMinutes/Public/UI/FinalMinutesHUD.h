#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FinalMinutesHUD.generated.h"


UCLASS()
class FINALMINUTES_API AFinalMinutesHUD : public AHUD
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnKillCountUpdated(int32 NewKillCount);
};
