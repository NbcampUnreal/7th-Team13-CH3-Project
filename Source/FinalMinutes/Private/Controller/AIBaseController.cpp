#include "Controller/AIBaseController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

const FName AAIBaseController::TargetKey(TEXT("TargetActor"));
const FName AAIBaseController::StateKey(TEXT("MonsterStats"));
const FName AAIBaseController::NoiseKey(TEXT("NoiseLocation"));

AAIBaseController::AAIBaseController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&AAIBaseController::OnTargetPerceptionUpdated);
	}
}

void AAIBaseController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (BTAsset)
	{
		RunBehaviorTree(BTAsset);
	}
	
	/*if (InPawn)
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
	);*/
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

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			if (Actor->ActorHasTag(TEXT("Player")))
			{
				BlackboardComp->SetValueAsObject(TargetKey, Actor);
				BlackboardComp->ClearValue(NoiseKey);
			}
		}
		else
		{
			BlackboardComp->ClearValue(TargetKey);
		}
		return;
	}
	
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			BlackboardComp->SetValueAsVector(
				NoiseKey,
				Stimulus.StimulusLocation);
		}
		return;
	}
}

/*
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
*/



