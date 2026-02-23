#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "Controller/PlayerCharacterController.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "AbilitySystem/Attributes/SensorAttributeSet.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Character/Components/CombatComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Camera/CameraComponent.h"

APlayerCharacter::APlayerCharacter()
{
	// ASC 달아주기
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
    // Tick 사용
    PrimaryActorTick.bCanEverTick = true;
    
	// 사용할 AttributeSet 설정
	CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	SensorAttributeSet = CreateDefaultSubobject<USensorAttributeSet>(TEXT("SensorAttributeSet"));
    WeaponAttributeSet = CreateDefaultSubobject<UWeaponAttributeSet>(TEXT("WeaponAttributeSet"));
    
    CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
    FollowCamera = FindComponentByClass<UCameraComponent>();
	// ASC초기화
	InitializeAbilitySystem();
  
  if (CombatComponent && DefaultWeaponTag.IsValid())
  {
     CombatComponent->EquipWeapon(DefaultWeaponTag);
  }
    
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
	    
	    // Zoom 태그용 델리게이트 바인딩
	    FGameplayTag ZoomTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsZooming"));
        
	    AbilitySystemComponent->RegisterGameplayTagEvent(
            ZoomTag, // 어떤 태그 감시?
            EGameplayTagEventType::NewOrRemoved // 새로 생기거나 제거될때 신호를 준다.
        ).AddUObject(this, &APlayerCharacter::OnZoomTagChanged); // 신호오면 알려줄 함수
	    
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
                EnhancedInput->BindAction(IA_Crouch, ETriggerEvent::Started, this, &APlayerCharacter::OnCrouch);
            }
            
            if (IA_Prone)
            {
                EnhancedInput->BindAction(IA_Prone, ETriggerEvent::Started, this, &APlayerCharacter::OnProne);
            }
            
            if (IA_Roll)
            {
                EnhancedInput->BindAction(IA_Roll, ETriggerEvent::Started, this, &APlayerCharacter::OnRoll);
            }
            
            if (IA_Sprint)
            {
                EnhancedInput->BindAction(IA_Sprint, ETriggerEvent::Started, this, &APlayerCharacter::StartSprint);
                EnhancedInput->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
            }
            
            if (PlayerController->EquipAction)
            {
                EnhancedInput->BindAction(PlayerController->EquipAction, ETriggerEvent::Started, this, &APlayerCharacter::Equip);
            }
            
            if (PlayerController->UnEquipAction)
            {
                EnhancedInput->BindAction(PlayerController->UnEquipAction, ETriggerEvent::Started, this, &APlayerCharacter::UnEquip);
            }
            
            if (IA_Reload)
            {
                EnhancedInput->BindAction(IA_Reload, ETriggerEvent::Started, this, &APlayerCharacter::OnReload);
            }
            
            if (IA_Attack)
            {
                EnhancedInput->BindAction(IA_Attack, ETriggerEvent::Started, this, &APlayerCharacter::OnAttackStarted);
                EnhancedInput->BindAction(IA_Attack, ETriggerEvent::Completed, this, &APlayerCharacter::OnAttackEnded);
            }
            
            
            if (IA_Zoom)
            {
                EnhancedInput->BindAction(IA_Zoom, ETriggerEvent::Started, this, &APlayerCharacter::OnZoomStarted);
                EnhancedInput->BindAction(IA_Zoom, ETriggerEvent::Completed, this, &APlayerCharacter::OnZoomEnded);
            }
        }
    }
}

void APlayerCharacter::Move(const FInputActionValue& value)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;
    if (!Controller) return;
    
    // 장전 중, 엎드린상태에서 공격중에는 움직일 수 없음
    FGameplayTag ReloadingTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsReloading"));
    FGameplayTag AttackingTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsAttacking"));
    FGameplayTag ProneTag      = FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning"));

    const bool bIsReloading = ASC->HasMatchingGameplayTag(ReloadingTag);
    const bool bIsAttacking = ASC->HasMatchingGameplayTag(AttackingTag);
    const bool bIsProning   = ASC->HasMatchingGameplayTag(ProneTag);
    
    if (bIsReloading || (bIsAttacking && bIsProning))
    {
        return;
    }
    
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
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;

    FGameplayTag CrouchStateTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsCrouching"));

    // 현재 앉은 상태 태그가 있는지 확인
    if (!ASC->HasMatchingGameplayTag(CrouchStateTag))
    {
        // 태그가 없다면 -> 앉기 실행
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Crouch")));
        ASC->TryActivateAbilitiesByTag(AbilityTags);
    }
    else
    {
        // 태그가 있다면 -> 앉기 이벤트 전송
        FGameplayEventData Payload;
        ASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(FName("State.Crouch.End")), &Payload);
    }
}

void APlayerCharacter::OnProne(const FInputActionValue& Value)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;

    FGameplayTag ProneStateTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning"));

    if (!ASC->HasMatchingGameplayTag(ProneStateTag))
    {
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Prone")));
        ASC->TryActivateAbilitiesByTag(AbilityTags);
    }
    else
    {
        FGameplayEventData Payload;
        ASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(FName("State.Prone.End")), &Payload);
    }
}

void APlayerCharacter::OnRoll(const FInputActionValue& Value)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;

    // 이미 구르는 중인지 확인
    FGameplayTag RollingTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsRolling"));
    
    // 구르는 중이 아닐 때만 구를수 있게
    if (!ASC->HasMatchingGameplayTag(RollingTag))
    {
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Roll")));
        
        // 상태 관리는 GA_Roll 내부의 PlayMontageAndWait에서
        ASC->TryActivateAbilitiesByTag(AbilityTags);
    }
}

void APlayerCharacter::GrantFireAbility()
{
}

void APlayerCharacter::OnReload(const FInputActionValue& Value)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;

    // 이미 장전 중인지 확인
    FGameplayTag ReloadTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsReloading"));
    
    // 장전중이 아닐때만 장전 가능하게
    if (!ASC->HasMatchingGameplayTag(ReloadTag))
    {
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Reload")));
        ASC->TryActivateAbilitiesByTag(AbilityTags);
    }
}

void APlayerCharacter::OnAttackStarted(const FInputActionValue& Value)
{
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Attack")));
    GetAbilitySystemComponent()->TryActivateAbilitiesByTag(AbilityTags);
    
    if (CombatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Firing Started..."));
        CombatComponent->Fire();
    }
}

void APlayerCharacter::OnAttackEnded(const FInputActionValue& Value)
{
    FGameplayEventData Payload;
    Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("State.Attack.End"));
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Payload.EventTag, Payload);
}

void APlayerCharacter::StartJump(const FInputActionValue& value)
{
    if (GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsRolling"))))
    {
        return;
    }
    
    Jump();
}

void APlayerCharacter::StopJump(const FInputActionValue& value)
{
    if (GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsRolling"))))
    {
        return;
    }
    
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
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Sprint")));
    GetAbilitySystemComponent()->TryActivateAbilitiesByTag(AbilityTags);
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    FGameplayEventData Payload;
    FGameplayTag StopTag = FGameplayTag::RequestGameplayTag(FName("Event.Montage.Sprint"));
    ASC->HandleGameplayEvent(StopTag, &Payload);
}

void APlayerCharacter::Equip(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Equip Weapon"));
    
    if (CombatComponent)
    {
        CombatComponent->EquipWeapon(DefaultWeaponTag);
    }
}

void APlayerCharacter::UnEquip(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("UnEquip Weapon"));
}

void APlayerCharacter::Interact(const FInputActionValue& Value)
{
}


void APlayerCharacter::OnZoomTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
    // NewCount는 이 줌 태그를 몇개 가지고 있는가 숫자, 즉 1이면 줌, 0이면 줌 아닌상태
    bIsZooming = (NewCount > 0);
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (FollowCamera)
    {
        float TargetFOV = bIsZooming ? 45.0f : 90.0f;
        float CurrentFOV = FollowCamera->FieldOfView;

        if (!FMath::IsNearlyEqual(CurrentFOV, TargetFOV, 0.1f))
        {
            float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 10.0f);
            FollowCamera->SetFieldOfView(NewFOV);
        }
        else if (CurrentFOV != TargetFOV)
        {
            FollowCamera->SetFieldOfView(TargetFOV);
        }
    }
}

void APlayerCharacter::OnZoomStarted(const FInputActionValue& Value)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;
    
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Zoom")));
    ASC->TryActivateAbilitiesByTag(AbilityTags);
}

void APlayerCharacter::OnZoomEnded(const FInputActionValue& Value)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;
    
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Zoom")));
    ASC->CancelAbilities(&AbilityTags);
}