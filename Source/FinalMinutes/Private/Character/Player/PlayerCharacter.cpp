#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "Controller/PlayerCharacterController.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "AbilitySystem/Attributes/SensorAttributeSet.h"

APlayerCharacter::APlayerCharacter()
{
	// ASC 달아주기
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	// 사용할 AttributeSet 설정
	CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	SensorAttributeSet = CreateDefaultSubobject<USensorAttributeSet>(TEXT("SensorAttributeSet"));
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// ASC초기화
	InitializeAbilitySystem();
}

void APlayerCharacter::InitializeAbilitySystem()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (APlayerCharacterController* PlayerController = Cast<APlayerCharacterController>(GetController()))
        {
            if (PlayerController->MoveAction)
            {
                EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
            }

            if (PlayerController->LookAction)
            {
                EnhancedInput->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
            }

            if (PlayerController->JumpAction)
            {
                EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::StartJump);
                EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopJump);
            }

            if (PlayerController->CrouchAction)
            {
                EnhancedInput->BindAction(PlayerController->CrouchAction, ETriggerEvent::Started, this, &APlayerCharacter::StartCrouch);
                EnhancedInput->BindAction(PlayerController->CrouchAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopCrouch);
            }
            
            if (PlayerController->SprintAction)
            {
                EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::StartSprint);
                EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
            }
            
            if (PlayerController->ProneAction)
            {
                EnhancedInput->BindAction(PlayerController->ProneAction, ETriggerEvent::Started, this, &APlayerCharacter::StartProne);
                EnhancedInput->BindAction(PlayerController->ProneAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopProne);
            }
            
            if (PlayerController->RollAction)
            {
                EnhancedInput->BindAction(PlayerController->RollAction, ETriggerEvent::Started, this, &APlayerCharacter::Roll);
            }
            
            if (PlayerController->EquipAction)
            {
                EnhancedInput->BindAction(PlayerController->EquipAction, ETriggerEvent::Started, this, &APlayerCharacter::Equip);
            }
            
            if (PlayerController->UnEquipAction)
            {
                EnhancedInput->BindAction(PlayerController->UnEquipAction, ETriggerEvent::Started, this, &APlayerCharacter::UnEquip);
            }
            
            if (PlayerController->ReloadAction)
            {
                EnhancedInput->BindAction(PlayerController->ReloadAction, ETriggerEvent::Started, this, &APlayerCharacter::Reload);
            }
            
            if (PlayerController->FireAction)
            {
                EnhancedInput->BindAction(PlayerController->FireAction, ETriggerEvent::Started, this, &APlayerCharacter::StartFire);
                EnhancedInput->BindAction(PlayerController->FireAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopFire);
            }
            
            if (PlayerController->InteractAction)
            {
                EnhancedInput->BindAction(PlayerController->InteractAction, ETriggerEvent::Started, this, &APlayerCharacter::Interact);
            }
        }
    }
}

void APlayerCharacter::Move(const FInputActionValue& value)
{
    if (!Controller) return;
    
    const FVector2D MoveInput = value.Get<FVector2D>();
    
    if (MoveInput.SquaredLength() < KINDA_SMALL_NUMBER) return;
    
    const FRotator Rotation = Controller->GetControlRotation();

    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    
    AddMovementInput(ForwardDirection, MoveInput.X);
    AddMovementInput(RightDirection, MoveInput.Y);
}

void APlayerCharacter::StartJump(const FInputActionValue& value)
{
    Jump();
}

void APlayerCharacter::StopJump(const FInputActionValue& value)
{
    StopJumping();
}

void APlayerCharacter::Look(const FInputActionValue& value)
{
    FVector2D LookInput = value.Get<FVector2D>();
    AddControllerYawInput(LookInput.X);
    // 상하 회전
    AddControllerPitchInput(LookInput.Y);
}

void APlayerCharacter::StartCrouch()
{
	UE_LOG(LogTemp, Warning, TEXT("Starting Crouch"));
	Crouch();
}
void APlayerCharacter::StopCrouch()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop Crouch"));
	UnCrouch();
}

void APlayerCharacter::StartSprint(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Start Sprint"));
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Stop Sprint"));
}

void APlayerCharacter::StartProne(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Start Prone"));
}

void APlayerCharacter::StopProne(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Stop Prone"));
}

void APlayerCharacter::Roll(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Rolling!"));
}

void APlayerCharacter::Equip(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Equip Weapon"));
}

void APlayerCharacter::UnEquip(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("UnEquip Weapon"));
}

void APlayerCharacter::Reload(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Reloading..."));
}

void APlayerCharacter::StartFire(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Firing Started"));
}

void APlayerCharacter::StopFire(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Firing Stopped"));
}

void APlayerCharacter::Interact(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Interaction Attempted"));
}