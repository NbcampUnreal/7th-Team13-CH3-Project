#include "Framewok/FinalMinutesGameState.h"

void AFinalMinutesGameState::AddKill()
{
	KillCount++;
	FOnKillCountChanged.Broadcast(KillCount);
}
	
void AFinalMinutesGameState::ResetKillCount()
{
	KillCount = 0;
	OnKillCountChanged.Broadcast(KillCount);
}