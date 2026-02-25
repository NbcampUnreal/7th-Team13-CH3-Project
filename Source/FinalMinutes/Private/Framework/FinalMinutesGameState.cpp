#include "Framework/FinalMinutesGameState.h"
#include "Framework/FinalMinutesGameMode.h"

AFinalMinutesGameState::AFinalMinutesGameState()
{
	KillCount = 0;
	//tick 돌아가게
	PrimaryActorTick.bCanEverTick = true;
}

void AFinalMinutesGameState::BeginPlay()
{
	Super::BeginPlay();
	
	KillCount = 0;
	OnKillCountChanged.Broadcast(KillCount);
	
	//테스트용 시간
	GameTime = 590.0f;
	
	//디버깅용
	UE_LOG(LogTemp, Warning, TEXT("게임 시작됨"));
}

void AFinalMinutesGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsGameCleared) return;

	//매 프레임마다 실제 흐른 시간을 더해줌
	GameTime += DeltaSeconds;
	
	//10분 도달 시 
	if (GameTime >= 600.0f)
	{
		GameTime = 600.0f; //10:00 값 고정시키기
		bIsGameCleared = true;
		AFinalMinutesGameMode* GM = Cast<AFinalMinutesGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->GameClear();
		}
	}
}

void AFinalMinutesGameState::AddKill(FGameplayTag MonsterTag)
{
	KillCount++;
	OnKillCountChanged.Broadcast(KillCount);
	//디버깅용
	UE_LOG(LogTemp, Warning, TEXT("킬 카운트 테스트 : %d"), KillCount);
}
	
void AFinalMinutesGameState::SetLoadedData(int32 LoadedKillCount, float LoadedTime)
{
	//킬카운트 덮어쓰기
	KillCount = LoadedKillCount;
	//불엉온 시간 덮어쓰기
	GameTime = LoadedTime;
	//생존시간 덮어쓰기
	BestSurviveTime = LoadedTime;
    
	// 점수가 바뀌었음을 UI에 방송
	OnKillCountChanged.Broadcast(KillCount);
	
}