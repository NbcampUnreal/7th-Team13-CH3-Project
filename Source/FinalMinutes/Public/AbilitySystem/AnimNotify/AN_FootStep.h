#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_FootStep.generated.h"

UCLASS()
class FINALMINUTES_API UAN_FootStep : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAN_FootStep();
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS")
	FGameplayTag FootStepCueTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS")
	float TraceDistance;
};
