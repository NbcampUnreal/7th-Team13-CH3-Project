#include "Task/BTTask_Attack.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

UBTTask_Attack::UBTTask_Attack()
{
    bNotifyTick = true;
    NodeName = TEXT("Monster Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Owner = OwnerComp.GetAIOwner();
    if (Owner == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    APawn* MyPawn = Owner->GetPawn();
    
    if (MyPawn == nullptr || AttackMontage == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    // 폰(Pawn) 내부에 있는 메시 컴포넌트를 직접 찾아봄
    if (const USkeletalMeshComponent* Mesh = MyPawn->FindComponentByClass<USkeletalMeshComponent>())
    {
        // 메시에서 애니메이션을 담당하는 인스턴스를 가져옴
        if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
        {
            // PlayAnimMontage가 Pawn에서는 지원되지 않아서 이렇게 구현함
            AnimInstance->Montage_Play(AttackMontage);
            return EBTNodeResult::InProgress;
        }
    }

    return EBTNodeResult::Failed;
}

void UBTTask_Attack::TickTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* Owner = OwnerComp.GetAIOwner();
    if (Owner == nullptr)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    APawn* MyPawn = Owner->GetPawn();
    
    if (MyPawn == nullptr)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    if (const USkeletalMeshComponent* Mesh = MyPawn->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
        {
            if (AnimInstance->Montage_IsPlaying(AttackMontage) == false)
            {
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            }
        }
    }
}


