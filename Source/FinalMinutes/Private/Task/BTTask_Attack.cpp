#include "Task/BTTask_Attack.h"
#include "Controller/AIBaseController.h"
#include "NavigationSystem.h"

/*
EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIBaseController* AIController = Cast<AAIBaseController>(OwnerComp.GetAIOwner());
    if (AIController == nullptr || AIController->GetPawn() == nullptr)
    {
        return EBTNodeResult::Aborted;
    }
    
    const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (AIController == nullptr || AttackMontage == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    AIController->PlayAnimMontage(AttackMontage);
    return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaTime)
{
    
}
*/
