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
	
	if (AAMonsterCharacter* PossessedMonster = Cast<AAMonsterCharacter>(InPawn))
	{
		PossessedMonster->bUseControllerRotationYaw = false;
		
		if (UFloatingPawnMovement* MovComp = Cast<UFloatingPawnMovement>(PossessedMonster->GetMovementComponent()))
		{
			/*MovComp->bOrientRotationToMovement = true; */
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

void AAIBaseController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* MyPawn = GetPawn();
	// 1. MyPawn이 유효하고 목표 회전값이 설정되었을 때만 실행
	if (MyPawn && !TargetRotation.IsNearlyZero())
	{
		FRotator CurrentRot = MyPawn->GetActorRotation();
        
		// 2. RInterpTo를 사용하여 현재 각도에서 목표 각도까지 부드럽게 보간 (속도 10.0f)
		FRotator SmoothRot = FMath::RInterpTo(CurrentRot, TargetRotation, DeltaTime, 10.0f);
        
		// MyPawn의 const를 제거하고 호출해야 에러가 나지 않습니다.
		MyPawn->SetActorRotation(SmoothRot);

		// 3. 목표 각도에 거의 도달하면 계산 중단
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
		BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
	}
	else
	{
		BlackboardComp->ClearValue(TEXT("TargetActor"));
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
		MoveToLocation(RandomLocation.Location);
       
		// 4. 즉시 회전시키는 대신, Tick에서 사용할 목표 각도만 업데이트합니다.
		FVector Direction = (RandomLocation.Location - GetPawn()->GetActorLocation()).GetSafeNormal();
		if (!Direction.IsNearlyZero())
		{
			TargetRotation = Direction.Rotation();
			TargetRotation.Pitch = 0.f;
			TargetRotation.Roll = 0.f;
		}
	}
}



