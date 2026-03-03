#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "FinalMinutesGameMode.generated.h"


UCLASS()
class FINALMINUTES_API AFinalMinutesGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AFinalMinutesGameMode();
	protected:
	
	virtual void BeginPlay() override;
	
public:
	
	//수타트
	UFUNCTION(BlueprintCallable, Category = "Game|Flow" )
	void GameStart();
	
	//일시정지
	UFUNCTION(BlueprintCallable, Category = "Game|Flow" )
	void GamePause(bool bIsPause);
	
	//게임 클리어
	UFUNCTION(BlueprintCallable, Category = "Game|Flow" )
	void GameClear();
	
	//게임 오버
	UFUNCTION(BlueprintCallable, Category = "Game|Flow" )
	void GameOver();
	
	//게임 나가기
	UFUNCTION(BlueprintCallable, Category = "Game|Flow" )
	void GameExit();
	
	//게임 로드했을 때 남은 시간 만큼 타이머 돌아가게 하는거
	UFUNCTION(BlueprintCallable, Category = "Game|Flow")
	void AdjustTimerAfterLoad(float LoadedTime);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameClearWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;
	
private:
	
	//타이머 관할 핸들러
	FTimerHandle TimerHandle;
	//생존시간 10분
	float TimeLimit = 600.0f;
};
