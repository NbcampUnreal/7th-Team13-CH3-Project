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
    
    GiveDefaultAbilities();
}

void APlayerCharacter::GiveDefaultAbilities()
{
    // ASC 확인
    if (!AbilitySystemComponent)
    {
        return;
    }

    // Authority 체크 (서버에서만 부여, 싱글플레이는 항상 true)
    if (!HasAuthority())
    {
        return;
    }

    // DefaultAbilities 배열 순회하며 부여
    for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
    {
        if (AbilityClass)
        {
            // Ability Spec 생성
            FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);

            // ASC에 Ability 부여
            AbilitySystemComponent->GiveAbility(AbilitySpec);
        }
    }
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

            if (IA_Crouch)
            {
                EnhancedInput->BindAction(IA_Crouch, ETriggerEvent::Triggered, this, &APlayerCharacter::OnCrouch);
                EnhancedInput->BindAction(IA_Crouch, ETriggerEvent::Completed, this, &APlayerCharacter::OnCrouch);
            }
            
            if (IA_Prone)
            {
                EnhancedInput->BindAction(IA_Prone, ETriggerEvent::Started, this, &APlayerCharacter::OnProne);
            }
            
            if (PlayerController->SprintAction)
            {
                EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::StartSprint);
                EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
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

void APlayerCharacter::OnCrouch(const FInputActionValue& Value)
{
    bool bIsPressed = Value.Get<bool>();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;
    if (bIsPressed)
    {
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Crouch")));
        ASC->TryActivateAbilitiesByTag((AbilityTags));
    }
    else
    {
        // 이벤트와 함께 전달되는 Payload(정보)
        // 이벤트 발생주체, 대상등이 들어있음
        FGameplayEventData Payload;
        ASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(FName("State.Crouch.End")), &Payload);
    }    
}

void APlayerCharacter::OnProne(const FInputActionValue& Value)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;

    FGameplayTag ProneStateTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning"));

    // 현재 엎드린 상태 태그가 있는지 확인
    if (!ASC->HasMatchingGameplayTag(ProneStateTag))
    {
        // 태그가 없다면 -> 엎드리기 실행
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Prone")));
        ASC->TryActivateAbilitiesByTag(AbilityTags);
    }
    else
    {
        // 태그가 있다면 -> 일어나기 이벤트 전송
        FGameplayEventData Payload;
        ASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(FName("State.Prone.End")), &Payload);
    }
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

void APlayerCharacter::StartSprint(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Start Sprint"));
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Stop Sprint"));
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