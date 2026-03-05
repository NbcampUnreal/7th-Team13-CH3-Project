#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Dead.generated.h"

UCLASS()
class FINALMINUTES_API UBTTask_Dead : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UBTTask_Dead();
    
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
    
protected:
    UPROPERTY(EditAnywhere, Category = "Dead")
    TObjectPtr<UAnimMontage> DeadMontage;
};
