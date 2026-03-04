#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Hit.generated.h"

UCLASS()
class FINALMINUTES_API UBTTask_Hit : public UBTTaskNode
{
    GENERATED_BODY()
    
public:
    UBTTask_Hit();
    
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
    
protected:
    UPROPERTY(EditAnywhere, Category="Attack")
    TObjectPtr<UAnimMontage> HitMontage;
};
