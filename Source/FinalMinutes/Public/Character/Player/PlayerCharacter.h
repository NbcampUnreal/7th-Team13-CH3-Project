#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "PlayerCharacter.generated.h"

class UWeaponAttributeSet;
class UAbilitySystemComponent;
class UCharacterAttributeSet;
class USensorAttributeSet;
class UGameplayAbility;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UGameplayEffect;
class UInventoryComponent;
class UPlayerStatusWidget;
class ABaseItem;

UCLASS()
class FINALMINUTES_API APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    APlayerCharacter();
    virtual void Tick(float DeltaTime) override;
    ABaseItem* GetFocusedItem() const { return FocusedItem.Get(); }
    
    virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    virtual void InitializeAbilitySystem();
    // virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

    FORCEINLINE class UCombatComponent* GetCombatComponent() const { return CombatComponent; }

protected:
#pragma region GameplayAbility관련
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UCombatComponent> CombatComponent;

    // Ability 부여 함수
    void GiveDefaultAbilities();
#pragma endregion

#pragma region Tag
    UPROPERTY(EditAnywhere, Category = "GAS | State Tags")
    FGameplayTag ReloadTag;
    UPROPERTY(EditAnywhere, Category = "GAS | State Tags")
    FGameplayTag AttackTag;
    UPROPERTY(EditAnywhere, Category = "GAS | State Tags")
    FGameplayTag ProneTag;
    UPROPERTY(EditAnywhere, Category = "GAS | State Tags")
    FGameplayTag DeathTag;
    UPROPERTY(EditAnywhere, Category = "GAS | State Tags")
    FGameplayTag CrouchTag;
    UPROPERTY(EditAnywhere, Category = "GAS | State Tags")
    FGameplayTag RollTag;
    UPROPERTY(EditAnywhere, Category = "GAS | State Tags")
    FGameplayTag ZoomTag;
    UPROPERTY(EditAnywhere, Category = "GAS | State Tags")
    FGameplayTag SATag;
    
    UPROPERTY(EditAnywhere, Category = "GAS | Ability Tags")
    FGameplayTag JumpAbilityTag;
    UPROPERTY(EditAnywhere, Category = "GAS | Ability Tags")
    FGameplayTag CrouchAbilityTag;
    UPROPERTY(EditAnywhere, Category = "GAS | Ability Tags")
    FGameplayTag ProneAbilityTag;
    UPROPERTY(EditAnywhere, Category = "GAS | Ability Tags")
    FGameplayTag RollAbilityTag;
    UPROPERTY(EditAnywhere, Category = "GAS | Ability Tags")
    FGameplayTag ReloadAbilityTag;
    UPROPERTY(EditAnywhere, Category = "GAS | Ability Tags")
    FGameplayTag AttackAbilityTag;
    UPROPERTY(EditAnywhere, Category = "GAS | Ability Tags")
    FGameplayTag SprintAbilityTag;
    UPROPERTY(EditAnywhere, Category = "GAS | Ability Tags")
    FGameplayTag InteractAbilityTag;
    UPROPERTY(EditAnywhere, Category = "GAS | Ability Tags")
    FGameplayTag ZoomAbilityTag;
    UPROPERTY(EditAnywhere, Category = "GAS | Ability Tags")
    FGameplayTag SAAbilityTag;
    
    UPROPERTY(EditAnywhere, Category = "GAS | Block Tags")
    FGameplayTagContainer MoveBlockTags;
    
    bool CanMove() const;
#pragma endregion
protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    TObjectPtr<UCharacterAttributeSet> CharacterAttributeSet;

    UPROPERTY()
    TObjectPtr<USensorAttributeSet> SensorAttributeSet;

    UPROPERTY()
    TObjectPtr<UWeaponAttributeSet> WeaponAttributeSet;

    /** 게임 시작 시 기본으로 장착할 보조무기(권총) 태그 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat | Init")
    FGameplayTag DefaultSecondaryWeaponTag;

    /** 만약 시작부터 주무기를 들려주고 싶다면 추가 (현재는 기획상 비워둠) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat | Init")
    FGameplayTag DefaultPrimaryWeaponTag;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FGameplayTag DefaultWeaponTag;
public:
    // 우리가 만든 인벤토리 컴포넌트를 담을 변수!
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInventoryComponent* InventoryComponent;
protected:
#pragma region 이동
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Move;

    UFUNCTION()
    void Move(const FInputActionValue& value);
#pragma endregion

#pragma region Look
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Look;

    UFUNCTION()
    void Look(const FInputActionValue& value);
#pragma endregion

#pragma region 점프
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Jump;

    UFUNCTION()
    void StartJump(const FInputActionValue& value);
    // UFUNCTION()
    // void StopJump(const FInputActionValue& value);
#pragma endregion

#pragma region 장비장착
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Equip;
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_UnEquip;

    UFUNCTION()
    void Equip(const FInputActionValue& value);
    UFUNCTION()
    void UnEquip(const FInputActionValue& value);
#pragma endregion

#pragma region 앉기
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Crouch;

    // 앉기
    void OnCrouch(const FInputActionValue& Value);
#pragma endregion

#pragma region 엎드리기
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Prone;
    // 엎드리기
    void OnProne(const FInputActionValue& Value);
#pragma endregion

#pragma region 구르기
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Roll;
    // 구르기
    void OnRoll(const FInputActionValue& Value);
#pragma endregion

#pragma region 장전
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Reload;

    void OnReload(const FInputActionValue& value);
#pragma endregion

    // 혹시 무기를 안들고 있을때 공격 로직이 필요할 것 같아서 Fire가 아닌 Attack으로 이름 지음
#pragma region 공격
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Attack;

    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Weapon1;

    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Weapon2;

    void OnAttackStarted(const FInputActionValue& value);
    void OnAttackEnded(const FInputActionValue& value);
    void OnWeapon1Input();
    void OnWeapon2Input();
#pragma endregion

#pragma region 전력질주
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
    TObjectPtr<UInputAction> IA_Sprint;

    UFUNCTION()
    void StartSprint(const FInputActionValue& value);
    UFUNCTION()
    void StopSprint(const FInputActionValue& value);
#pragma endregion

#pragma region 줌
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Input")
    UInputAction* IA_Zoom;

    // 우클릭을 눌렀을 때 (어빌리티 실행)
    void OnZoomStarted(const FInputActionValue& Value);

    // 우클릭을 뗐을 때 (어빌리티 종료/입력 해제 신호)
    void OnZoomEnded(const FInputActionValue& Value);

    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FollowCamera;

    // 줌 상태 관리 변수
    bool bIsZooming;

    void OnZoomTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
#pragma endregion

#pragma region 상호작용
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Input")
    UInputAction* IA_Interact;

    UFUNCTION()
    void Interact(const FInputActionValue& value);
    
    void UpdateItemOutline();

    UPROPERTY(EditDefaultsOnly, Category="Interact|Outline")
    float InteractDistance = 700.f;
#pragma endregion

#pragma region 스태미너
    UPROPERTY(EditDefaultsOnly, Category = "Abilities")
    TSubclassOf<UGameplayEffect> StaminaRegenEffectClass;
#pragma endregion
    
#pragma region 인벤토리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Input")
    UInputAction* IA_Inventory;

    void ToggleInventoryInput();
#pragma endregion	

#pragma region 반동
    FVector2D TargetRecoil;  // 최종적으로 도달해야 할 목표 반동량
    FVector2D CurrentRecoil; // 현재 프레임까지 적용된 반동량

    UPROPERTY(EditAnywhere, Category = "Recoil")
    float RecoilInterpSpeed = 15.0f; // 총이 위로 튀는 속도 (높을수록 빠름)

    UPROPERTY(EditAnywhere, Category = "Recoil")
    float RecoveryInterpSpeed = 10.0f;
#pragma endregion 
    
#pragma region 일시정지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    FString IAPausePath = TEXT("/Game/FinalMinutes/Inputs/IA_Pause.IA_Pause");
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FString PauseMenuPath = TEXT("/Game/FinalMinutes/UI/WBP_Menu.WBP_Menu_C");
    
    UPROPERTY()
    class UInputAction* IA_Pause;

    // 에디터에서 WBP_PauseMenu를 할당할 클래스 변수
    UPROPERTY()
    TSubclassOf<class UUserWidget> PauseMenuClass;

    // 현재 생성된 위젯을 저장해둘 변수 (중복 생성 방지용)
    UPROPERTY()
    class UUserWidget* PauseMenuInstance;

    // P 누르면 실행될 함수
    void TogglePause();
#pragma endregion	
    
#pragma region 특수능력
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Input")
    UInputAction* IA_SpecialAbility;
    
    void OnSpecialAbility();
#pragma endregion
public:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    virtual void PossessedBy(AController* NewController) override;

    UPROPERTY()
    TObjectPtr<UPlayerStatusWidget> MainHUD = nullptr;

    void CacheMainHUD();
    
private:
    TWeakObjectPtr<ABaseItem> FocusedItem;
};
