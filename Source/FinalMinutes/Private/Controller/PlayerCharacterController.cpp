#include "Controller/PlayerCharacterController.h"
#include "EnhancedInputSubsystems.h"

APlayerCharacterController::APlayerCharacterController()
	: InputMappingContext(nullptr),
	  MoveAction(nullptr),
	  JumpAction(nullptr),
	  LookAction(nullptr),
	  SprintAction(nullptr),
	  CrouchAction(nullptr),
	  ProneAction(nullptr),
	  RollAction(nullptr),
	  EquipAction(nullptr),
	  UnEquipAction(nullptr),
	  ReloadAction(nullptr),
	  FireAction(nullptr),
	  InteractAction(nullptr)
{
}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();
	
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				SubSystem->AddMappingContext(InputMappingContext, 0);	
			}
		}
	}
}





	










