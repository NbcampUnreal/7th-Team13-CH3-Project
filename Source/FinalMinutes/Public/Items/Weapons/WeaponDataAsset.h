#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "WeaponDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FString WeaponName;

	//무기 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float WeaponDamage;
};