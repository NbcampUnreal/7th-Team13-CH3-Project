#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemDataSubsytem.generated.h"


UCLASS()
class FINALMINUTES_API UItemDataSubsytem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemData)
	UDataTable* MasterItemTable;
	
	UFUNCTION(BlueprintCallable, Category = ItemData)
	class UWeaponDataAsset* GetWeaponData(FGameplayTag Tag);
	
};
