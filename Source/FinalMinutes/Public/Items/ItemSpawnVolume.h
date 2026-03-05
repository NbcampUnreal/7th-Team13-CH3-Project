#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Items/ItemData.h"
#include "ItemSpawnVolume.generated.h"

class ABaseItem;
class UBoxComponent;

UCLASS()
class FINALMINUTES_API AItemSpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemSpawnVolume();

protected:
	virtual void BeginPlay() override;

public:	
	// ====== 스폰 설정 ======
	UPROPERTY(EditAnywhere, Category="Spawn")
	UDataTable* ItemDataTable = nullptr;

	/* 아이템 스폰 횟수 */
	UPROPERTY(EditAnywhere, Category="Spawn", meta=(ClampMin="0"))
	int32 SpawnCount = 10;

	/* 겹치지 않게 최소 거리 */
	UPROPERTY(EditAnywhere, Category="Spawn", meta=(ClampMin="0.0"))
	float MinDistance = 80.f;
	
	/* 아이템 1개당 위치 찾기 재시도 횟수 */
	UPROPERTY(EditAnywhere, Category="Spawn", meta=(ClampMin="1"))
	int32 MaxTriesPerItem = 80;

	/* BeginPlay 때 자동 스폰 */
	UPROPERTY(EditAnywhere, Category="Spawn")
	bool bSpawnOnBeginPlay = true;
	
	// ===== 계단/단차 필터 파라미터 =====
	UPROPERTY(EditAnywhere, Category="Spawn|Filter")
	float MaxStepHeight = 30.f;   // cm: 이 이상 높이 차면 '계단/단차'로 보고 금지 (25~40 추천)

	UPROPERTY(EditAnywhere, Category="Spawn|Filter")
	float SampleRadius = 45.f;    // cm: 주변 샘플링 거리 (30~60 추천)

	UPROPERTY(EditAnywhere, Category="Spawn|Filter")
	float FloorNormalZMin = 0.98f; // 1.0에 가까울수록 평평한 바닥만 허용 (0.95~0.99 추천)
private:
	// 이미 스폰한 위치들
	UPROPERTY()
	TArray<FVector> SpawnedLocations;
	
	UPROPERTY()
	UBoxComponent* SpawnBoxComp = nullptr;

private:
	// 스폰 실행
	void SpawnItems();
	void SpawnOneItem();
	
	//SpawnBox 안인지 체크(XY만)
	bool IsPointInsideSpawnBox(const FVector& WorldPoint) const;
	
	// Reachable NavMesh + SpawnBox + 계단 필터
	bool FindNavMeshGroundLocation(FVector& OutLoc) const;
	
	// 기존 스폰 위치와 충분히 떨어져 있는지
	bool IsFarEnough(const FVector& Candidate) const;
	// DT에서 아이템 랜덤(가중치) 선택
	bool PickRandomItemFromDT(FName& OutItemID, TSubclassOf<ABaseItem>& OutClass) const;
	
	// 계단/단차 필터용
	bool TraceFloorAt(const FVector& WorldPos, FVector& OutHitPoint, FVector& OutNormal) const;
	bool IsValidFlatFloorAt(const FVector& NavPoint, FVector& OutFloorPoint) const;
};
