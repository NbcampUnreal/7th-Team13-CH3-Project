#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify_PauseClothingSimulation.h"
#include "GameplayTagContainer.h"
#include "AN_MFootStep.generated.h"


UCLASS()
class FINALMINUTES_API UAN_MFootStep : public UAnimNotify_PauseClothingSimulation
{
    GENERATED_BODY()
public:
    UAN_MFootStep();
	
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS")
    FGameplayTag FootStepCueTag;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS")
    float TraceDistance;
};
