#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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
	
	//일시정지 인벤토리?
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
	
private:
	
	//타이머 관할 핸들러
	FTimerHandle TimerHandle;
	//생존시간 10분
	float TimeLimit = 600.0f;
};
