#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"

UCLASS()
class FINALMINUTES_API UBTTask_Attack : public UBTTaskNode
{
    GENERATED_BODY()
    
/*public:
    // Begin(Enter)
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
    
    // End(Exit)
    virtual void TickTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaTime) override;
    
protected:
    UPROPERTY(EditAnywhere, Category="Attack")
    TObjectPtr<UAnimMontage> AttackMontage;*/
};
