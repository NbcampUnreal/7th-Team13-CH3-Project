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

void AFinalMinutesGameState::AddKill()
{
	KillCount++;
	OnKillCountChanged.Broadcast(KillCount);
	//디버깅용
	UE_LOG(LogTemp, Warning, TEXT("킬 카운트 테스트 : %s"), KillCount);
}
	
