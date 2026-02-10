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
	UPROPERTY(EditAnywhere, Category = "ItemData")
	TObjectPtr<UDataTable> WeaponDataTable;
	UPROPERTY(EditAnywhere, Category = "MonsterData")
	TObjectPtr<UDataTable> MonsterDataTable;
	
	UFUNCTION(BlueprintCallable, Category = "ItemData")
	FWeaponData GetWeaponData(FGameplayTag Tag);
	UFUNCTION(BlueprintCallable, Category = "MonsterData")
	FMonsterData GetMonsterData(FGameplayTag Tag);
};