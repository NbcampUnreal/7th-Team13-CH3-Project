#include "Task/UBTTask_FindRandomLocation.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Controller/AIBaseController.h"

UBTTask_FindRandomLocation::UBTTask_FindRandomLocation()
{
    NodeName = TEXT("Find Random Location");
    
    PatrolLocationKey.AddVectorFilter(this,
        GET_MEMBER_NAME_CHECKED(UBTTask_FindRandomLocation, PatrolLocationKey)
        );
}

EBTNodeResult::Type UBTTask_FindRandomLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIBaseController* AIController = Cast<AAIBaseController>(OwnerComp.GetAIOwner());
    if (AIController == nullptr || AIController->GetPawn() == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    const FVector Origin = AIController->GetPawn()->GetActorLocation();
    FNavLocation RandomLocation;
    
    const bool bFound = NavSystem->GetRandomReachablePointInRadius(Origin, SearchRadius, RandomLocation);
    if (bFound)
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(
            PatrolLocationKey.SelectedKeyName,
            RandomLocation.Location);
        return EBTNodeResult::Succeeded;
    }
    
    return EBTNodeResult::Failed;
}
