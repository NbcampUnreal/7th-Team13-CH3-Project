#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UBTTask_FindRandomLocation.generated.h"

UCLASS()
class FINALMINUTES_API UBTTask_FindRandomLocation : public UBTTaskNode
{
    GENERATED_BODY()
    
public:
    UBTTask_FindRandomLocation();
    
    // Begin(Enter)
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
    
protected:
    UPROPERTY(EditAnywhere, Category="Blackboard")
    FBlackboardKeySelector PatrolLocationKey;
    
    UPROPERTY(EditAnywhere, Category="Blackboard")
    float SearchRadius = 500.0f;
};
