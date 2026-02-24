#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacterController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class FINALMINUTES_API APlayerCharacterController : public APlayerController
{
	GENERATED_BODY()
	
public:
	APlayerCharacterController();
	
	virtual void BeginPlay() override;
	
	// 액션들, 나중에 DataAsset으로 정의할 수도 있다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputMappingContext* InputMappingContext;
};
