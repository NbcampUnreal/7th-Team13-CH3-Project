#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AIBaseController.generated.h"

class UAIPerceptionComponent;

UCLASS()
class FINALMINUTES_API AAIBaseController : public AAIController
{
	GENERATED_BODY()

public:
	AAIBaseController();

	virtual void OnPossess(APawn* InPawn) override;
	
protected:
	virtual void Tick(float DeltaTime) override;
	
protected:
	UPROPERTY(EditAnywhere, Category="AI")
	TObjectPtr<class UBehaviorTree> BTAsset;
	
	UPROPERTY(EditAnywhere, Category="AI")
	TObjectPtr<class UBlackboardData> BBAsset;
	
	UPROPERTY(EditDefaultsOnly, Category="AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;
	
	// 블랙보드 키 이름
	static const FName TargetKey;
	static const FName StateKey;
	static const FName NoiseKey;
	
private:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	// void MoveToRandomLocation();
	
	UPROPERTY(EditAnywhere, Category = "AI")
	float MoveRadius = 600.0f;
	
	
	FRotator TargetRotation;
	UPROPERTY(EditAnywhere, Category = "Rotate")
	float RotationSpeed = 90.0f;
};
