#include "Controller/AIBaseController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Monster/AMonsterCharacter.h"
#include "Perception/AIPerceptionComponent.h"

const FName AAIBaseController::TargetKey(TEXT("Target"));
const FName AAIBaseController::StateKey(TEXT("MonsterStats"));

AAIBaseController::AAIBaseController()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	
}

void AAIBaseController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (BTAsset)
	{
		RunBehaviorTree(BTAsset);
	}
	
	if (AAMonsterCharacter* PossessedMonster = Cast<AAMonsterCharacter>(InPawn))
	{
		PossessedMonster->bUseControllerRotationYaw = false;

		
		if (UFloatingPawnMovement* MovComp = Cast<UFloatingPawnMovement>(PossessedMonster->GetMovementComponent()))
		{
			MovComp->UFloatingPawnMovement = true; 
		}
	}
	
	GetWorldTimerManager().SetTimer(
		RandomMoveTimer,
		this,
		&AAIBaseController::MoveToRandomLocation,
		3.0f,
		true,
		1.0f
	);
}

void AAIBaseController::BeginPlay()
{
	Super::BeginPlay();
	
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&AAIBaseController::OnTargetPerceptionUpdated);
	}
}

void AAIBaseController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
	if (BlackboardComp == nullptr)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
	}
	else
	{
		BlackboardComp->ClearValue(TEXT("TargetActor"));
	}
}

void AAIBaseController::MoveToRandomLocation()
{
	const APawn* MyPawn = GetPawn();
	if (MyPawn == nullptr) return;
	
	const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem == nullptr) return;
	
	FNavLocation RandomLocation;
	bool bFound = NavSystem->GetRandomReachablePointInRadius(
		MyPawn->GetActorLocation(),
		MoveRadius,
		RandomLocation
		);
	
	if (bFound)
	{
		MoveToLocation(RandomLocation.Location);
	}
}



