#include "Framework/FinalMinutesGameState.h"

void AFinalMinutesGameState::AddKill()
{
	KillCount++;
	OnKillCountChanged.Broadcast(KillCount);
}
	
