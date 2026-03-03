#include "Task/UBTTask_FindRandomLocation.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Controller/AIBaseController.h"

UBTTask_FindRandomLocation::UBTTask_FindRandomLocation()
{
    // 현재 노드의 이름
    NodeName = TEXT("Find Random Location");
    
    // 이 노드가 블랙보드와 연동될 때, "Vector"타입의 데이터만 저장할 수 있도록 필터링
    PatrolLocationKey.AddVectorFilter(this,
        GET_MEMBER_NAME_CHECKED(UBTTask_FindRandomLocation, PatrolLocationKey)
        );
}

// BT에서 해당 노드에 도달했을 때, 실제로 수행할 동작을 정의하는 가상 함수
EBTNodeResult::Type UBTTask_FindRandomLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // AIController와 AIController가 조종하고 있는 Pawn이 있는지 확인
    AAIBaseController* AIController = Cast<AAIBaseController>(OwnerComp.GetAIOwner());
    if (AIController == nullptr || AIController->GetPawn() == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    // UE5의 길 찾기 시스템. UNavigationSystemV1 에셋을 현재 월드에서 가져옴
    const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem == nullptr)
    {
        return EBTNodeResult::Failed;
    }
    
    // AI의 현재 위치를 기준점(origin)으로 잡고, 무작위 좌표를 RandomLocation에 저장
    const FVector Origin = AIController->GetPawn()->GetActorLocation();
    FNavLocation RandomLocation;
    
    // 제한된 거리(SearchRadius) 내에서 AI가 걸어서 도달 가능한 무작위 지점을 서칭
    // 서칭해서 값이 참(도달 가능으로 판단)이면, 찾은 좌표를 블랙보드의 특정 키(PatrolLocation key)에 기록
    if (const bool bFound = NavSystem->GetRandomReachablePointInRadius(Origin, SearchRadius, RandomLocation))
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(
            PatrolLocationKey.SelectedKeyName,
            RandomLocation.Location);
        return EBTNodeResult::Succeeded;
    }
    
    // 예외 처리
    return EBTNodeResult::Failed;
}
