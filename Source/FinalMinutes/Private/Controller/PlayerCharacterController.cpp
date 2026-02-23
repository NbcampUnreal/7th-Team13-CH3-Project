#include "Controller/PlayerCharacterController.h"
#include "EnhancedInputSubsystems.h"

APlayerCharacterController::APlayerCharacterController()
	: InputMappingContext(nullptr)
{
}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;
	UEnhancedInputLocalPlayerSubsystem* SubSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!SubSystem) return;
	if (InputMappingContext)
	{
		SubSystem->AddMappingContext(InputMappingContext, 0);	
	}
	
}





	










