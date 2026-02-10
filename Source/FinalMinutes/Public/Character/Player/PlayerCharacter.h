#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "PlayerCharacter.generated.h"

class UAbilitySystemComponent;
class UCharacterAttributeSet;
class USensorAttributeSet;

UCLASS()
class FINALMINUTES_API APlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void InitializeAbilitySystem();
	
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
	
	
	// ACharacter가 이미 가지고 있음
	// UFUNCTION()
	// void Crouch(const FInputActionValue& value);
	
	void StartCrouch();
	void StopCrouch();
	
	
public:	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
