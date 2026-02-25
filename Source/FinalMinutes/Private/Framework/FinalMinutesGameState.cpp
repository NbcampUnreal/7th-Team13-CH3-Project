#include "Framework/FinalMinutesGameState.h"


AFinalMinutesGameState::AFinalMinutesGameState()
{
	KillCount = 0;
}

void AFinalMinutesGameState::BeginPlay()
{
	Super::BeginPlay();
	
	KillCount = 0;
	OnKillCountChanged.Broadcast(KillCount);
	//디버깅용
	UE_LOG(LogTemp, Warning, TEXT("아따 게임 시작 됐십니더"));
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
	//생존시간 덮어쓰기
	BestSurviveTime = LoadedTime;
    
	// 점수가 바뀌었음을 UI에 방송
	OnKillCountChanged.Broadcast(KillCount);
	
}