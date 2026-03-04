#include "Task/BTTask_Dead.h"

#include "Controller/AIBaseController.h"
#include "GameFramework/PawnMovementComponent.h"

UBTTask_Dead::UBTTask_Dead()
{
    bNotifyTick = true;
    NodeName = TEXT("Monster Dead");
}

EBTNodeResult::Type UBTTask_Dead::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Owner = OwnerComp.GetAIOwner();
    if (Owner == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    APawn* MyPawn = Owner->GetPawn();
    
    if (MyPawn == nullptr || DeadMontage == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    if (UPawnMovementComponent* MoveComp = MyPawn->FindComponentByClass<UPawnMovementComponent>())
    {
        // 1. 물리적 속도 즉시 제거 및 이동 기능 정지
        MoveComp->StopMovementImmediately();
        MoveComp->Deactivate(); // 컴포넌트 자체를 비활성화
    }

    // 2. 애니메이션 재생 및 삭제 예약
    if (USkeletalMeshComponent* Mesh = MyPawn->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
        {
            // 몽타주를 재생하고 전체 재생 시간(초)을 가져옵니다.
            float PlayTime = AnimInst->Montage_Play(DeadMontage);

            // 4. [핵심] 애니메이션이 끝나기 직전에 액터를 삭제합니다.
            // 재생 시간의 90~95% 시점에 사라지게 하면 벌떡 일어날 틈이 없습니다.
            MyPawn->SetLifeSpan(PlayTime * 0.95f);
        }
    }

    // 이 노드에 계속 머물게 하여 다른 이동 명령을 차단합니다.
    return EBTNodeResult::InProgress;
}

