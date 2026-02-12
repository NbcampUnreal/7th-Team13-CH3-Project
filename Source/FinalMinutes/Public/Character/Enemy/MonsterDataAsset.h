#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MonsterDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FMonsterData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster") 
	FGameplayTag MonsterTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") 
	float MaxHp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") 
	float AttackDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Defence;
	
};