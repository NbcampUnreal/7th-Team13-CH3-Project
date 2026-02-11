#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FinalMinutesGameInstance.generated.h"

class UItemDataSubsystem;
class USoundSubsystem;
class USaveSubsystem;

UCLASS()
class FINALMINUTES_API UFinalMinutesGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	// 서브시스템 접근용 
	UFUNCTION(BlueprintCallable, Category = "Subsystem")
	UItemDataSubsystem* GetItemSubsystem() const;

	UFUNCTION(BlueprintCallable, Category = "Subsystem")
	USoundSubsystem* GetSoundSubsystem() const;
};