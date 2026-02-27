#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "PlayerStatusWidget.generated.h"

class UAbilitySystemComponent;
class UCombatComponent;
class UProgressBar;
class UTextBlock;

UCLASS()
class FINALMINUTES_API UPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:	
	UFUNCTION(BlueprintCallable)
	void InitWithASC(UAbilitySystemComponent* InASC);
	
protected:
	virtual void NativeDestruct() override;
	
private:
	// Attribute 바인딩/해제
    void BindCallbacks();
    void UnbindCallbacks();
	
	// CombatComponent 바인딩/해제 (무기 아이콘용)
	void BindCombatCallbacks();
	void UnbindCombatCallbacks();
	
	UFUNCTION()
	void HandleActiveWeaponTagChanged(FGameplayTag WeaponTag);
	
	// UI 갱신
    void UpdateHealth(float Current);
    void UpdateStamina(float Current);
	void UpdateAmmo(float Current);
	void UpdateMaxAmmo(float Max);
	
	// Attribute 델리게이트 핸들러
    void OnHealthChanged(const FOnAttributeChangeData& Data);
    void OnStaminaChanged(const FOnAttributeChangeData& Data);
	void OnAmmoChanged(const FOnAttributeChangeData& Data);
	void OnMaxAmmoChanged(const FOnAttributeChangeData& Data);
	
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> ASC;

	// 무기 아이콘 이벤트를 받을 CombatComponent(소유자 캐릭터의 컴포넌트)
	UPROPERTY()
	TObjectPtr<UCombatComponent> BoundCombatComp;
	
	// Attribute 델리게이트 핸들
    FDelegateHandle HealthChangedHandle;
    FDelegateHandle StaminaChangedHandle;
	FDelegateHandle AmmoChangedHandle;
	FDelegateHandle MaxAmmoChangedHandle;

    float MaxHealth = 100.f;
    float MaxStamina = 100.f;

protected:
    // UMG 디자이너에서 ProgressBar 이름을 정확히 맞추고 "Is Variable" 체크
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UProgressBar> PB_Health;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> TXT_Health;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UProgressBar> PB_Stamina;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> TXT_Ammo;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> TXT_MaxAmmo;
	
	// 무기 타입 태그가 바뀌었을 때 UI 갱신용
	UFUNCTION(BlueprintImplementableEvent)
	void OnActiveWeaponTagChanged(FGameplayTag WeaponTag); 
};
