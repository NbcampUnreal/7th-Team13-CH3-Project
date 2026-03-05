#include "Items/ItemSpawnVolume.h"
#include "Items/BaseItem.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "AI/Navigation/NavigationTypes.h"

AItemSpawnVolume::AItemSpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AItemSpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
	// BP에 붙인 BoxCollision 1개를 스폰 범위로 사용
	SpawnBoxComp = FindComponentByClass<UBoxComponent>();
	if (!SpawnBoxComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemSpawnVolume] BoxComponent가 없습니다. BP에 Box Collision을 추가하세요."));
		return;
	}

	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemSpawnVolume] ItemDataTable이 NULL 입니다."));
	}

	if (bSpawnOnBeginPlay)
	{
		SpawnItems();
	}
}

void AItemSpawnVolume::SpawnItems()
{
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemSpawnVolume] ItemDataTable is NULL"));
		return;
	}

	// 겹치지 않게 이미 아이템 스폰한 위치 저장 배열
	SpawnedLocations.Reset(); 

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		SpawnOneItem();
	}
}
void AItemSpawnVolume::SpawnOneItem()
{
	// 반경 안에서 랜덤 좌표를 뽑음
	FVector SpawnLoc;
	if (!FindNavMeshGroundLocation(SpawnLoc)) return;

	FName ItemID = NAME_None;
	TSubclassOf<ABaseItem> ItemClass = nullptr;

	if (!PickRandomItemFromDT(ItemID, ItemClass)) return;
	if (!ItemClass) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABaseItem* Spawned = GetWorld()->SpawnActor<ABaseItem>(ItemClass, SpawnLoc, FRotator::ZeroRotator, Params);
	if (!Spawned) return;

	Spawned->ItemID = ItemID;
	SpawnedLocations.Add(SpawnLoc);
}

// SpawnBox 안인지 체크 (XY만)
bool AItemSpawnVolume::IsPointInsideSpawnBox(const FVector& WorldPoint) const
{
	if (!SpawnBoxComp) return false;

	const FVector Ext = SpawnBoxComp->GetScaledBoxExtent();
	const FVector Local = SpawnBoxComp->GetComponentTransform().InverseTransformPosition(WorldPoint);

	return (FMath::Abs(Local.X) <= Ext.X &&
			FMath::Abs(Local.Y) <= Ext.Y);
}

bool AItemSpawnVolume::FindNavMeshGroundLocation(FVector& OutLoc) const
{
    UWorld* World = GetWorld();
    if (!World || !SpawnBoxComp) return false;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys) return false;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return false;

    const FVector PlayerLoc = PlayerPawn->GetActorLocation();

    // SpawnBox 전체에서 균등 랜덤 후보 생성용
    const FVector Ext = SpawnBoxComp->GetScaledBoxExtent();
    const FTransform BoxT = SpawnBoxComp->GetComponentTransform();

    // 후보를 NavMesh 위로 올릴 때 사용하는 허용 범위
    const FVector ProjectExtent(200.f, 200.f, 600.f);

    // ✅ 층 스냅(1층으로 떨어지는 문제) 방지용 허용 높이 차이
    const float MaxZDiff = 100.f; // 층고에 따라 200~600 조절

    for (int32 Try = 0; Try < MaxTriesPerItem; ++Try)
    {
        // 1) SpawnBox 안에서 랜덤 후보점(균등) - ✅ Z도 랜덤
        const FVector Local(
            FMath::FRandRange(-Ext.X, Ext.X),
            FMath::FRandRange(-Ext.Y, Ext.Y),
            FMath::FRandRange(-Ext.Z, Ext.Z)
        );
        const FVector Candidate = BoxT.TransformPosition(Local);

        // 2) 후보점을 NavMesh 위로 투영
        FNavLocation NavLoc;
        if (!NavSys->ProjectPointToNavigation(Candidate, NavLoc, ProjectExtent))
        	continue;

        // ✅ 추가: 후보 높이와 너무 다른 층으로 붙으면(스냅) 버림
        if (FMath::Abs(NavLoc.Location.Z - Candidate.Z) > MaxZDiff)
        	continue;

        // 3) SpawnBox 안인지(안전)
        if (!IsPointInsideSpawnBox(NavLoc.Location))
        	continue;

        // ✅ Path 안정화: 플레이어 위치도 NavMesh로 투영 + Context로 PlayerPawn 전달
        FNavLocation PlayerNav;
        if (!NavSys->ProjectPointToNavigation(PlayerLoc, PlayerNav, ProjectExtent))
        	continue;

        UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(
            World,
            PlayerNav.Location,
            NavLoc.Location,
            PlayerPawn
        );

        if (!Path || !Path->IsValid() || Path->IsPartial())
        	continue;

        // 5) 계단/단차 금지 + 바닥에 붙이기
        FVector FloorPoint;
        if (!IsValidFlatFloorAt(NavLoc.Location, FloorPoint))
        	continue;

        // 6) 겹침 방지
        if (!IsFarEnough(FloorPoint))
        	continue;

        OutLoc = FloorPoint;
        return true;
    }

    return false;
}
// 새로 스폰하려는 위치가, 기존에 스폰된 아이템들과 너무 가까운지 검사
bool AItemSpawnVolume::IsFarEnough(const FVector& Candidate) const
{
	if (MinDistance <= 0.f) return true;

	for (const FVector& P : SpawnedLocations)
	{
		if (FVector::Dist2D(P, Candidate) < MinDistance)
			return false;
	}
	return true;
}

// DT 가중치 랜덤 선택
bool AItemSpawnVolume::PickRandomItemFromDT(FName& OutItemID, TSubclassOf<ABaseItem>& OutClass) const
{
	if (!ItemDataTable) return false;

	TArray<FItemData*> Rows;
	ItemDataTable->GetAllRows(TEXT("Spawn"), Rows);
	if (Rows.Num() == 0) return false;

	// 전체 가중치 합
	float Total = 0.f;
	for (auto* R : Rows)
	{
		if (!R || !R->ItemActorClass) continue;
		Total += FMath::Max(0.f, R->SpawnWeight);
	}
	if (Total <= 0.f) return false;

	// 0~Total 사이 랜덤
	float Roll = FMath::FRandRange(0.f, Total);
	float Acc = 0.f;

	// 누적합으로 선택
	for (auto* R : Rows)
	{
		if (!R || !R->ItemActorClass) continue;

		Acc += FMath::Max(0.f, R->SpawnWeight);
		if (Roll <= Acc)
		{
			OutItemID = R->ItemID;
			OutClass  = R->ItemActorClass;
			return true;
		}
	}
	return false;
}

bool AItemSpawnVolume::TraceFloorAt(const FVector& WorldPos, FVector& OutHitPoint, FVector& OutNormal) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	FHitResult Hit;

	// 아주 짧게 위->아래로 바닥만 확인 (너무 길면 다른 층/구조물 맞을 수 있음)
	const FVector Start = WorldPos + FVector(0, 0, 80.f);
	const FVector End   = WorldPos - FVector(0, 0, 250.f);

	FCollisionQueryParams Q;
	Q.AddIgnoredActor(this);

	// Trace Channel 추가 안 하기로 했으니 Visibility 사용
	if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Q))
		return false;

	OutHitPoint = Hit.ImpactPoint;
	OutNormal   = Hit.ImpactNormal;
	return true;
}

bool AItemSpawnVolume::IsValidFlatFloorAt(const FVector& NavPoint, FVector& OutFloorPoint) const
{
	// 중심 바닥 찍기
	FVector CenterPoint, CenterNormal;
	if (!TraceFloorAt(NavPoint, CenterPoint, CenterNormal))
		return false;

	// 경사면 제외(선택이지만 추천)
	// 계단의 각 발판은 보통 수평이라 이 조건만으로 계단이 완전히 걸러지진 않지만,
	// 경사로/비탈 스폰 방지에 도움 됨.
	if (CenterNormal.Z < FloorNormalZMin)
		return false;

	// 주변 4방향 높이 비교로 “단차(계단)” 감지
	const FVector Offsets[4] =
	{
		FVector(SampleRadius, 0, 0),
		FVector(-SampleRadius, 0, 0),
		FVector(0, SampleRadius, 0),
		FVector(0, -SampleRadius, 0)
	};

	for (const FVector& Off : Offsets)
	{
		FVector P, N;
		if (!TraceFloorAt(NavPoint + Off, P, N))
			return false;

		// 주변과 높이차가 크면 계단/단차 근처로 판단 → 금지
		if (FMath::Abs(P.Z - CenterPoint.Z) > MaxStepHeight)
			return false;
	}

	// 최종 스폰 좌표는 “중심 바닥점”
	OutFloorPoint = CenterPoint;
	return true;
}



