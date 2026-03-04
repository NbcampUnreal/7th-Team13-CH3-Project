#include "Task/BTTask_Hit.h"
#include "Controller/AIBaseController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Hit::UBTTask_Hit()
{
    bNotifyTick = true;
    NodeName = TEXT("Monster Hit");
}

EBTNodeResult::Type UBTTask_Hit::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Owner = OwnerComp.GetAIOwner();
    if (Owner == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    APawn* MyPawn = Owner->GetPawn();
    
    if (MyPawn == nullptr || HitMontage == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    if (USkeletalMeshComponent* Mesh = MyPawn->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
        {
            AnimInst->Montage_Play(HitMontage);
            
            if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
            {
                BB->SetValueAsBool(TEXT("bIsHit"), false);
            }
        }
    }
    
    return EBTNodeResult::Succeeded;
}

