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
#include "GameFramework/CharacterMovementComponent.h"
#include "Framework/FinalMinutesGameMode.h"
#include "Items/BaseItem.h"

APlayerCharacter::APlayerCharacter()
{
    // ASC 달아주기
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

    // Tick 사용
    PrimaryActorTick.bCanEverTick = true;

    // 사용할 AttributeSet 설정
    CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
    SensorAttributeSet = CreateDefaultSubobject<USensorAttributeSet>(TEXT("SensorAttributeSet"));
    WeaponAttributeSet = CreateDefaultSubobject<UWeaponAttributeSet>(TEXT("WeaponAttributeSet"));


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
    DefaultSecondaryWeaponTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Type.Pistol"));
    
    // 실제 게임할 때는 이 줄은 주석 처리
    // DefaultPrimaryWeaponTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Type.Rifle"));
    
    // 약간의 지연 후 장착 (서브시스템 로딩 대기)
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this]()
    {
        if (CombatComponent)
        {
            if (DefaultSecondaryWeaponTag.IsValid()) CombatComponent->EquipWeapon(DefaultSecondaryWeaponTag);
            if (DefaultPrimaryWeaponTag.IsValid()) CombatComponent->EquipWeapon(DefaultPrimaryWeaponTag);
        }
    }, 0.1f, false);

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
        if (!AbilityClass) continue;
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
    
    AbilitySystemComponent->RegisterGameplayTagEvent(
        ZoomTag, // 어떤 태그 감시?
        EGameplayTagEventType::AnyCountChange // 새로 생기거나 제거될때 신호를 준다.
    ).AddUObject(this, &APlayerCharacter::OnZoomTagChanged); // 신호오면 알려줄 함수

    // 캐릭터는 스태미너가 계속해서 찬다.
    if (!StaminaRegenEffectClass) return;
    FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
    ContextHandle.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
        StaminaRegenEffectClass, 1.0f, ContextHandle);
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
    
    // 1번 키 (주무기)
    if (IA_Weapon1)
    {
        EnhancedInput->BindAction(IA_Weapon1, ETriggerEvent::Started, this, &APlayerCharacter::OnWeapon1Input);
    }

    // 2번 키 (보조무기)
    if (IA_Weapon2)
    {
        EnhancedInput->BindAction(IA_Weapon2, ETriggerEvent::Started, this, &APlayerCharacter::OnWeapon2Input);
    }
    if (IA_Inventory)
    {
        EnhancedInput->BindAction(IA_Inventory, ETriggerEvent::Started, this, &APlayerCharacter::ToggleInventoryInput);
    }
    
    if (!IA_Pause)
    {
        IA_Pause = LoadObject<UInputAction>(nullptr, *IAPausePath);
    }
    
    if (IA_Pause)
    {
        EnhancedInput->BindAction(IA_Pause, ETriggerEvent::Started, this, &APlayerCharacter::TogglePause);
    }

    if (IA_SpecialAbility)
    {
        EnhancedInput->BindAction(IA_SpecialAbility, ETriggerEvent::Started, this, &APlayerCharacter::OnSpecialAbility);
    }
}

bool APlayerCharacter::CanMove() const
{
    if (!AbilitySystemComponent) return false;
    if (AbilitySystemComponent->HasAnyMatchingGameplayTags(MoveBlockTags)) return false;
    
    bool bIsAttacking = AbilitySystemComponent->HasMatchingGameplayTag(AttackTag);
    bool bIsProning = AbilitySystemComponent->HasMatchingGameplayTag(ProneTag);
    if (bIsAttacking && bIsProning) return false;
    return true;
}


void APlayerCharacter::Move(const FInputActionValue& value)
{
    if (!Controller) return;
    if (!CanMove()) return;
    
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
    // 현재 앉은 상태 태그가 있는지 확인
    if (!AbilitySystemComponent->HasMatchingGameplayTag(CrouchTag))
    {
        // 태그가 없다면 -> 앉기 실행
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(CrouchAbilityTag);
        AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
    }
    else
    {
        static FGameplayTagContainer CrouchBlockTags;
    
        if (CrouchBlockTags.IsEmpty())
        {
            CrouchBlockTags.AddTag(ReloadTag);
        }
    
        if (AbilitySystemComponent->HasAnyMatchingGameplayTags(CrouchBlockTags)) return;
        // 태그가 있다면 -> 앉기 이벤트 전송
        FGameplayEventData Payload;
        AbilitySystemComponent->HandleGameplayEvent(
            FGameplayTag::RequestGameplayTag(FName("State.Crouch.End")),
            &Payload);
    }
}

void APlayerCharacter::OnProne(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    if (!AbilitySystemComponent->HasMatchingGameplayTag(ProneTag))
    {
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(ProneAbilityTag);
        AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
    }
    else
    {
        static FGameplayTagContainer ProneBlockTags;
    
        if (ProneBlockTags.IsEmpty())
        {
            ProneBlockTags.AddTag(ReloadTag);
        }
    
        if (AbilitySystemComponent->HasAnyMatchingGameplayTags(ProneBlockTags)) return;
        FGameplayEventData Payload;
        AbilitySystemComponent->HandleGameplayEvent
        (FGameplayTag::RequestGameplayTag(FName("State.Prone.End")), &Payload);
    }
}

void APlayerCharacter::OnRoll(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    
    // 제자리 구르기 방지
    if (GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero())
    {
        return;
    }
    
    // 이미 구르는 중인지 확인
    // 구르는 중이 아닐 때만 구를수 있게
    if (!AbilitySystemComponent->HasMatchingGameplayTag(RollTag))
    {
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(RollAbilityTag);

        // 상태 관리는 GA_Roll 내부의 PlayMontageAndWait에서
        AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
    }
}

void APlayerCharacter::OnReload(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    // 이미 장전 중인지 확인
    // 장전중이 아닐때만 장전 가능하게
    if (!AbilitySystemComponent->HasMatchingGameplayTag(ReloadTag))
    {
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(ReloadAbilityTag);
        AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
    }
}

void APlayerCharacter::OnAttackStarted(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(AttackAbilityTag);

    AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
}

void APlayerCharacter::OnAttackEnded(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    FGameplayTagContainer CancelTags;
    CancelTags.AddTag(AttackAbilityTag);
    
    AbilitySystemComponent->CancelAbilities(&CancelTags);
}

void APlayerCharacter::OnWeapon1Input()
{
    if (CombatComponent) CombatComponent->SwapToSlot(EWeaponSlot::Primary);
}

void APlayerCharacter::OnWeapon2Input()
{
    if (CombatComponent) CombatComponent->SwapToSlot(EWeaponSlot::Secondary);
}

void APlayerCharacter::StartJump(const FInputActionValue& value)
{
    FGameplayTagContainer JumpTags;
    JumpTags.AddTag(JumpAbilityTag);
    AbilitySystemComponent->TryActivateAbilitiesByTag(JumpTags);
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
    AbilityTags.AddTag(SprintAbilityTag);
    AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    FGameplayEventData Payload;
    FGameplayTag StopTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Montage.Sprint")));
    AbilitySystemComponent->HandleGameplayEvent(StopTag, &Payload);
}

void APlayerCharacter::Equip(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Equip Weapon"));

    if (CombatComponent)
    {
        CombatComponent->EquipWeapon(DefaultPrimaryWeaponTag);
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
    AbilityTags.AddTag(InteractAbilityTag);

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
    UpdateItemOutline();
}

void APlayerCharacter::OnZoomStarted(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(ZoomAbilityTag);
    AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
}

void APlayerCharacter::OnZoomEnded(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(ZoomAbilityTag);
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

void APlayerCharacter::TogglePause()
{
    AFinalMinutesGameMode* GM = Cast<AFinalMinutesGameMode>(GetWorld()->GetAuthGameMode());

    //이미 메뉴가 켜져 있다면 지우고 일시정지 풀기
    if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
    {
        PauseMenuInstance->RemoveFromParent();
        PauseMenuInstance = nullptr;

        if (GM) GM->GamePause(false);
        return; 
    }

    //위젯 클래스가 없다면 문자열 경로로 로드
    if (!PauseMenuClass)
    {
        PauseMenuClass = LoadClass<UUserWidget>(nullptr, *PauseMenuPath);
    }

    //로드에 성공했다면 화면에 띄우고 게임 멈추기
    if (PauseMenuClass)
    {
        PauseMenuInstance = CreateWidget<UUserWidget>(GetWorld(), PauseMenuClass);
        if (PauseMenuInstance)
        {
            PauseMenuInstance->AddToViewport();
            
            if (GM) GM->GamePause(true);
        }
    }
}

void APlayerCharacter::UpdateItemOutline()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    FVector CamLoc;
    FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);

    const FVector Start = CamLoc;
    const FVector End   = Start + CamRot.Vector() * InteractDistance;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractTrace), false, this);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, Start, End, ECC_Visibility, Params
    );

    ABaseItem* NewItem = bHit ? Cast<ABaseItem>(Hit.GetActor()) : nullptr;
    ABaseItem* OldItem = FocusedItem.Get();

    if (NewItem == OldItem) return;

    // 이전 아이템: 아웃라인 끄고 + 위젯 숨기기
    if (OldItem)
    {
        OldItem->SetOutline(false);
        OldItem->SetPromptVisible(false);
    }

    // 새 아이템: 아웃라인 켜고 + 위젯 보이기
    if (NewItem)
    {
        NewItem->SetOutline(true);
        NewItem->SetPromptVisible(true);
    };
    FocusedItem = NewItem;
}

void APlayerCharacter::OnSpecialAbility()
{
    if (!AbilitySystemComponent) return;
    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(SAAbilityTag);
    
    if (AbilitySystemComponent->HasMatchingGameplayTag(SATag))
    {
        AbilitySystemComponent->CancelAbilities(&AbilityTags);
    }
    else
    {
        AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
    }
}