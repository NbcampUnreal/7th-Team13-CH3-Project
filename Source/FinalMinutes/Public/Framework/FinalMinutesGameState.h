#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"
#include "FinalMinutesGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillCountChanged, int32, NewKillCount);

UCLASS()
class FINALMINUTES_API AFinalMinutesGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	AFinalMinutesGameState();
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(BlueprintCallable, Category="GameState")
	void AddKill(FGameplayTag MonsterTag);
	
	// 킬 수 확인용
	UFUNCTION(BlueprintCallable, Category = "Data")
	int32 GetKillCount() const { return KillCount; }
	
	//세이브파일에서 데이터를 불러와 덮어쓸 때 쓸 함수
	UFUNCTION(BlueprintCallable, Category = "GameState")
	void SetLoadedData(int32 LoadedKillCount, float LoadedTime);
	
	//시간당 페이즈로 나눠 브금을 바꾸는 이벤트 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Audio")
	void ChangeBGM(int32 PhaseIndex);
	
	//UI에서 접근 할 최고 생존 시간
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	float BestSurviveTime = 0.0f;
	
	// 현재까지 살아남은 시간 (초 단위)
	UPROPERTY(BlueprintReadOnly, Category = "GameMode")
	float GameTime = 0.0f;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnKillCountChanged OnKillCountChanged;
	
	//현재 재생중인 BGM 페이즈를 기억할 변수 ( 0= 재생x, 1=초반,2=중반,3=후반)
	int32 CurrentBGMPhase = 0;

	//블루프린트에서 음악을 쓰게하는 변수 3개
	UPROPERTY(EditAnywhere, Category = "BGM")
	class USoundBase* BGM_Phase1;

	UPROPERTY(EditAnywhere, Category = "BGM")
	class USoundBase* BGM_Phase2;

	UPROPERTY(EditAnywhere, Category = "BGM")
	class USoundBase* BGM_Phase3;
	
	UPROPERTY(BlueprintReadWrite, Category = "GameRules")
	bool bIsGameStarted = false;

private:
	int32 KillCount = 0;
	bool bIsGameCleared = false;
};