#include "Controller/AIBaseController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Monster/AMonsterCharacter.h"
#include "Perception/AIPerceptionComponent.h"

const FName AAIBaseController::TargetKey(TEXT("TargetActor"));
const FName AAIBaseController::StateKey(TEXT("MonsterStats"));

AAIBaseController::AAIBaseController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	
}

void AAIBaseController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (BTAsset)
	{
		RunBehaviorTree(BTAsset);
	}
	
	if (InPawn)
	{
		InPawn->bUseControllerRotationYaw = false;
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

void AAIBaseController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	APawn* MyPawn = GetPawn();
	if (MyPawn && !TargetRotation.IsNearlyZero())
	{
		
		FRotator CurrentRot = MyPawn->GetActorRotation();
		FRotator SmoothRot = FMath::RInterpTo(CurrentRot, TargetRotation, DeltaTime, 10.0f);
		
		MyPawn->SetActorRotation(SmoothRot);
		
		if (CurrentRot.Equals(TargetRotation, 1.0f))
		{
			TargetRotation = FRotator::ZeroRotator;
		}
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
		GetWorldTimerManager().PauseTimer(RandomMoveTimer);
		BlackboardComp->SetValueAsObject(TargetKey, Actor);
	}
	else
	{
		BlackboardComp->ClearValue(TargetKey);
		GetWorldTimerManager().UnPauseTimer(RandomMoveTimer);
	}
}

void AAIBaseController::MoveToRandomLocation()
{
	APawn* MyPawn = GetPawn();
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
		FVector Direction = (RandomLocation.Location - GetPawn()->GetActorLocation()).GetSafeNormal();
		if (!Direction.IsNearlyZero())
		{
			TargetRotation = Direction.Rotation();
			TargetRotation.Pitch = 0.f;
			TargetRotation.Roll = 0.f;
		}
		
		MoveToLocation(RandomLocation.Location);
	}
}



