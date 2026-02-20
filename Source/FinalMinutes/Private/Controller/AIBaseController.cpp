#include "Controller/AIBaseController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
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




