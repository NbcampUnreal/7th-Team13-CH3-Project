#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

class UPlayerStatusWidget;
class UAbilitySystemComponent;

UCLASS()
class FINALMINUTES_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

private:
	// PC의 Pawn 변경 이벤트로 호출될 함수
	UFUNCTION()
	void OnPawnChanged(APawn* NewPawn);

	void TryCreateWidgetFromPawn(APawn* NewPawn);
	void CreateAndInit(UAbilitySystemComponent* ASC);
	
private:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UPlayerStatusWidget> StatusWidgetClass;

	UPROPERTY()
	TObjectPtr<UPlayerStatusWidget> StatusWidget;
};
