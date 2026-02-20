#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "PlayerStatusWidget.generated.h"

class UAbilitySystemComponent;
class UProgressBar;

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
    void BindCallbacks();
    void UnbindCallbacks();

    void UpdateHealth(float Current);
    void UpdateStamina(float Current);

    void OnHealthChanged(const FOnAttributeChangeData& Data);
    void OnStaminaChanged(const FOnAttributeChangeData& Data);
	
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> ASC;

    FDelegateHandle HealthChangedHandle;
    FDelegateHandle StaminaChangedHandle;

    float MaxHealth = 100.f;
    float MaxStamina = 100.f;

protected:
    // UMG 디자이너에서 ProgressBar 이름을 정확히 맞추고 "Is Variable" 체크
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UProgressBar> PB_Health;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UProgressBar> PB_Stamina;
};
