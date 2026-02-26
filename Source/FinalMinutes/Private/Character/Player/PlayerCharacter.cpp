#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "AbilitySystem/Attributes/SensorAttributeSet.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Character/Components/CombatComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "UI/PlayerHUD.h"
#include "UI/PlayerStatusWidget.h"
#include "Components/InventoryComponent.h"

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
    bIsZooming = false;
    
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
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

void APlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    CacheMainHUD();
}

void APlayerCharacter::CacheMainHUD()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    APlayerHUD* HUD = Cast<APlayerHUD>(PC->GetHUD());
    if (!HUD) return;

    MainHUD = HUD->GetMainHUDWidget();
}

void APlayerCharacter::GiveDefaultAbilities()
{
    // Authority 체크 (서버에서만 부여, 싱글플레이는 항상 true)
    if (!HasAuthority()) return;
    if (!AbilitySystemComponent) return;

    // DefaultAbilities 배열 순회하며 부여
    for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
    {
        if (!AbilityClass) return;
        // Ability Spec 생성
        FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);

        // ASC에 Ability 부여
        AbilitySystemComponent->GiveAbility(AbilitySpec);
    }
}

void APlayerCharacter::InitializeAbilitySystem()
{
    if (!AbilitySystemComponent) return;
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	    
	// Zoom 태그용 델리게이트 바인딩
	FGameplayTag ZoomTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsZooming"));
    
	AbilitySystemComponent->RegisterGameplayTagEvent(
        ZoomTag, // 어떤 태그 감시?
        EGameplayTagEventType::NewOrRemoved // 새로 생기거나 제거될때 신호를 준다.
    ).AddUObject(this, &APlayerCharacter::OnZoomTagChanged); // 신호오면 알려줄 함수
	
	// 캐릭터는 스태미너가 계속해서 찬다.
	if (!StaminaRegenEffectClass) return;
	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StaminaRegenEffectClass, 1.0f, ContextHandle);
	if (!SpecHandle.IsValid()) return;
	
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput) return;
    
    if (IA_Move)
    {
        EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
    }

    if (IA_Look)
    {
        EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
    }

    if (IA_Jump)
    {
        EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Started, this, &APlayerCharacter::StartJump);
        EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Completed, this, &APlayerCharacter::StopJump);
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
    
    if (IA_Equip)
    {
        EnhancedInput->BindAction(IA_Equip, ETriggerEvent::Started, this, &APlayerCharacter::Equip);
    }
    
    if (IA_UnEquip)
    {
        EnhancedInput->BindAction(IA_UnEquip, ETriggerEvent::Started, this, &APlayerCharacter::UnEquip);
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
    
    if (IA_Interact)
    {
        EnhancedInput->BindAction(IA_Interact, ETriggerEvent::Started, this, &APlayerCharacter::Interact);
    }
    
    if (IA_Zoom)
    {
        EnhancedInput->BindAction(IA_Zoom, ETriggerEvent::Started, this, &APlayerCharacter::OnZoomStarted);
        EnhancedInput->BindAction(IA_Zoom, ETriggerEvent::Completed, this, &APlayerCharacter::OnZoomEnded);
    }
    if (IA_Inventory)
    {
        EnhancedInput->BindAction(IA_Inventory, ETriggerEvent::Started, this, &APlayerCharacter::ToggleInventoryInput);
    }
}

void APlayerCharacter::Move(const FInputActionValue& value)
{
    if (!AbilitySystemComponent) return;
    if (!Controller) return;
    
    // 장전 중, 엎드린상태에서 공격중에는 움직일 수 없음
    FGameplayTag ReloadingTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsReloading"));
    FGameplayTag AttackingTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsAttacking"));
    FGameplayTag ProneTag      = FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning"));

    const bool bIsReloading = AbilitySystemComponent->HasMatchingGameplayTag(ReloadingTag);
    const bool bIsAttacking = AbilitySystemComponent->HasMatchingGameplayTag(AttackingTag);
    const bool bIsProning   = AbilitySystemComponent->HasMatchingGameplayTag(ProneTag);
    
    if (bIsReloading || (bIsAttacking && bIsProning)) return;
    
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
    if (!AbilitySystemComponent) return;
    FGameplayTag CrouchStateTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsCrouching"));

    // 현재 앉은 상태 태그가 있는지 확인
    if (!AbilitySystemComponent->HasMatchingGameplayTag(CrouchStateTag))
    {
        // 태그가 없다면 -> 앉기 실행
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Crouch")));
        AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
    }
    else
    {
        // 태그가 있다면 -> 앉기 이벤트 전송
        FGameplayEventData Payload;
        AbilitySystemComponent->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(FName("State.Crouch.End")), &Payload);
    }
}

void APlayerCharacter::OnProne(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    FGameplayTag ProneStateTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning"));

    if (!AbilitySystemComponent->HasMatchingGameplayTag(ProneStateTag))
    {
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Prone")));
        AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
    }
    else
    {
        FGameplayEventData Payload;
        AbilitySystemComponent->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(FName("State.Prone.End")), &Payload);
    }
}

void APlayerCharacter::OnRoll(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    // 이미 구르는 중인지 확인
    FGameplayTag RollingTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsRolling"));
    
    // 구르는 중이 아닐 때만 구를수 있게
    if (!AbilitySystemComponent->HasMatchingGameplayTag(RollingTag))
    {
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Roll")));
        
        // 상태 관리는 GA_Roll 내부의 PlayMontageAndWait에서
        AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
    }
}

void APlayerCharacter::GrantFireAbility()
{
}

void APlayerCharacter::OnReload(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    // 이미 장전 중인지 확인
    FGameplayTag ReloadTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsReloading"));
    
    // 장전중이 아닐때만 장전 가능하게
    if (!AbilitySystemComponent->HasMatchingGameplayTag(ReloadTag))
    {
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Reload")));
        AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
    }
}

void APlayerCharacter::OnAttackStarted(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Attack")));
    AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
    
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
    if (!AbilitySystemComponent ||AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsRolling"))))
    {
        return;
    }
    
    Jump();
}

void APlayerCharacter::StopJump(const FInputActionValue& value)
{
    
    if (!AbilitySystemComponent || AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsRolling"))))
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
    if (!AbilitySystemComponent) return;
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Sprint")));
    AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    FGameplayEventData Payload;
    FGameplayTag StopTag = FGameplayTag::RequestGameplayTag(FName("Event.Montage.Sprint"));
    AbilitySystemComponent->HandleGameplayEvent(StopTag, &Payload);
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
    if (!AbilitySystemComponent) return;
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Interact")));
    
    AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
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
    if (!AbilitySystemComponent) return;
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Zoom")));
    AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
}

void APlayerCharacter::OnZoomEnded(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Zoom")));
    AbilitySystemComponent->CancelAbilities(&AbilityTags);
}

void APlayerCharacter::ToggleInventoryInput()
{
    if (MainHUD)
    {
        // MainHUDWidget블루프린트 안에 ToggleInventoryWindow이름을 가진 함수가 있는지 검색합니다.
        FName const FunctionName = TEXT("ToggleInventoryWindow");
        
        // ToggleInventoryWindow함수가 있따면
        if (UFunction* Function = MainHUD->FindFunction(FunctionName))
        {
            // 그 함수를 호출합니다.
            MainHUD->ProcessEvent(Function, nullptr);
        }
    }
}