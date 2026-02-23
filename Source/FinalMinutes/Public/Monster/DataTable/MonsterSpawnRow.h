#pragma once

#include "CoreMinimal.h"
#include "MonsterSpawnRow.generated.h"

USTRUCT(BlueprintType)
struct FMonsterSpawnRow :public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MonsterName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> MonsterCalss;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Spawnchance;
};
