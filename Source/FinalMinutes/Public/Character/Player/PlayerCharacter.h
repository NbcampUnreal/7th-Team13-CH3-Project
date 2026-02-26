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

UCLASS()
class FINALMINUTES_API APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void InitializeAbilitySystem();
	
	FORCEINLINE class UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	
protected:
	#pragma region GameplayAbility관련
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	// Ability 부여 함수
	void GiveDefaultAbilities();
#pragma endregion
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	TObjectPtr<UCharacterAttributeSet> CharacterAttributeSet;
    
	UPROPERTY()
	TObjectPtr<USensorAttributeSet> SensorAttributeSet;
	
	UPROPERTY()
	TObjectPtr<UWeaponAttributeSet> WeaponAttributeSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UCombatComponent> CombatComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FGameplayTag DefaultWeaponTag;
public :
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
	UFUNCTION()
	void StopJump(const FInputActionValue& value);
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
	
	void GrantFireAbility();
// 혹시 무기를 안들고 있을때 공격 로직이 필요할 것 같아서 Fire가 아닌 Attack으로 이름 지음
#pragma region 공격
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
	TObjectPtr<UInputAction> IA_Attack;
	
	void OnAttackStarted(const FInputActionValue& value);
	void OnAttackEnded(const FInputActionValue& value);
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
	
public:	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
protected:
	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY()
	TObjectPtr<UPlayerStatusWidget> MainHUD = nullptr;

	void CacheMainHUD();
};
