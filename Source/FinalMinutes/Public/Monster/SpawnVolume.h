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
	
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void UpdateWave(float CurrentTime);
	
	// 에디터에서 엔딩 때, 몬스터가 이동할 최종 목적지를 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ending", meta = (MakeEditWidget = true))
	FVector TargetEndingLocation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	USceneComponent* Scene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	UBoxComponent* SpawnBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	UDataTable* MonsterDataTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float SpawnInterval;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float StopSpawnTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 MaxSpawnCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float SpawnInit;
	
	float LastSpawnCheckTime = -1.0f;
	
	void SpawnRandomMonster();
	FMonsterSpawnRow* GetRandomMonster() const;
	
private:
	void SpawnMonsters(int32 Count);
	void SpawnMonster(TSubclassOf<AActor> MonsterClass);
};
