#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FinalMinutes/Public/Character/Enemy/MonsterDataAsset.h"
#include "FinalMinutes/Public/Items/Weapons/WeaponDataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemDataSubsystem.generated.h"


UCLASS()
class FINALMINUTES_API UItemDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	UItemDataSubsystem();
	
	UPROPERTY()
	TObjectPtr<UDataTable> WeaponDataTable;
	UPROPERTY()
	TObjectPtr<UDataTable> MonsterDataTable;
	
	UFUNCTION(BlueprintCallable, Category = "ItemData")
	FWeaponData GetWeaponData(FGameplayTag Tag);
	UFUNCTION(BlueprintCallable, Category = "MonsterData")
	FMonsterData GetMonsterData(FGameplayTag Tag);
	
};