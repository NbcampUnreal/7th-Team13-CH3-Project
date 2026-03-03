#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SpawnVolume.generated.h"

struct FMonsterSpawnRow;
class USceneComponent;
class UBoxComponent;

UCLASS()
class FINALMINUTES_API ASpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	ASpawnVolume();
	
	void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	USceneComponent* Scene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	UBoxComponent* SpawnBox;
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<AActor> MonsterToSpawn;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	UDataTable* MonsterDataTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float SpawnRate;
	
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnRandomMonster();
	
	FMonsterSpawnRow* GetRandomMonster() const;
	void SpawnMonster(TSubclassOf<AActor> MonsterClass);
	
	FTimerHandle SpawnTimer;
	
};
