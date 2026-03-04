#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

class UPlayerStatusWidget;
class UAbilitySystemComponent;
class UUserWidget;

UCLASS()
class FINALMINUTES_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	UPlayerStatusWidget* GetMainHUDWidget();
	
	UFUNCTION(BlueprintCallable, Category="UI")
	void ShowDamageNumber(int32 Damage);
	
	UFUNCTION(BlueprintCallable, Category="UI")
	void ShowKillPlusOne();
	
	UFUNCTION(BlueprintCallable, Category="UI")
	void ShowGameStartMessage();
	
protected:
	virtual void BeginPlay() override;

private:
	// PC의 Pawn 변경 이벤트로 호출될 함수
	UFUNCTION()
	void OnPawnChanged(APawn* NewPawn);

	void TryCreateWidgetFromPawn(APawn* NewPawn);
	void CreateAndInit(UAbilitySystemComponent* ASC);
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UPlayerStatusWidget> MainHUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UPlayerStatusWidget> MainHUDWidget; 
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> GameClearWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> ResultWidget;
};
