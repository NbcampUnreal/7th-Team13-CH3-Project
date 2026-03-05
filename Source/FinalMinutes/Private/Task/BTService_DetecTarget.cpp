#include "Task/BTService_DetectTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTService_DetectTarget::UBTService_DetectTarget()
{
    NodeName = TEXT("Detect Target");
    Interval = 0.5f;
    RandomDeviation = 0.1f;

    TargetActorKey.AddObjectFilter(
        this, 
        GET_MEMBER_NAME_CHECKED(UBTService_DetectTarget, TargetActorKey),
        AActor::StaticClass());
}

void UBTService_DetectTarget::TickNode(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    
    const AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController == nullptr || AIController->GetPawn() == nullptr)
    {
        return;
    }

    APawn* MyPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (MyPawn == nullptr)
    {
        OwnerComp.GetBlackboardComponent()->ClearValue(TargetActorKey.SelectedKeyName);
        return;
    }

    const float Distance = FVector::Dist(
        AIController->GetPawn()->GetActorLocation(),
        MyPawn->GetActorLocation());

    if (Distance <= DetectRadius)
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsObject(TargetActorKey.SelectedKeyName, MyPawn);
    }
    else
    {
        OwnerComp.GetBlackboardComponent()->ClearValue(TargetActorKey.SelectedKeyName);
    }
}
