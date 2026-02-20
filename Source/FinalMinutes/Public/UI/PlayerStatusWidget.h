#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "PlayerStatusWidget.generated.h"

class UAbilitySystemComponent;
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
	// 바인딩/해제
    void BindCallbacks();
    void UnbindCallbacks();

	// UI 갱신
    void UpdateHealth(float Current);
    void UpdateStamina(float Current);

	void UpdateAmmo(float Current);
	void UpdateMaxAmmo(float Max);
	
	// 델리게이트 핸들러
    void OnHealthChanged(const FOnAttributeChangeData& Data);
    void OnStaminaChanged(const FOnAttributeChangeData& Data);
	
	void OnAmmoChanged(const FOnAttributeChangeData& Data);
	void OnMaxAmmoChanged(const FOnAttributeChangeData& Data);
	
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> ASC;

	// 델리게이트 핸들
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
};
