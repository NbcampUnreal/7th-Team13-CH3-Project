#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "PlayerCharacter.generated.h"

class UAbilitySystemComponent;
class UCharacterAttributeSet;
class USensorAttributeSet;
class UGameplayAbility;
class UInputAction;

UCLASS()
class FINALMINUTES_API APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void InitializeAbilitySystem();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	// Ability 부여 함수
	void GiveDefaultAbilities();
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UCharacterAttributeSet> CharacterAttributeSet;
    
	UPROPERTY()
	TObjectPtr<USensorAttributeSet> SensorAttributeSet;
	
protected:
	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void StartJump(const FInputActionValue& value);
	UFUNCTION()
	void StopJump(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);
	UFUNCTION()
	void StartProne(const FInputActionValue& value);
	UFUNCTION()
	void StopProne(const FInputActionValue& value);
	UFUNCTION()
	void Roll(const FInputActionValue& value);
	UFUNCTION()
	void Equip(const FInputActionValue& value);
	UFUNCTION()
	void UnEquip(const FInputActionValue& value);
	UFUNCTION()
	void Reload(const FInputActionValue& value);
	
	UFUNCTION()
	void StartFire(const FInputActionValue& value); // 발사
	UFUNCTION()
	void StopFire(const FInputActionValue& value); // 혹시 마우스 호버로 발사한다면
	
	UFUNCTION()
	void Interact(const FInputActionValue& value);
	
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Input")
	TObjectPtr<UInputAction> IA_Crouch;

	void OnCrouch(const FInputActionValue& Value);
	
	void StartCrouch();
	void StopCrouch();
	
	
public:	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
