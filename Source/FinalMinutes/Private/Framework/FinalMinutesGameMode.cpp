#include "Framework/FinalMinutesGameMode.h"
#include "Framework/FinalMinutesGameState.h"

AFinalMinutesGameMode::AFinalMinutesGameMode()
{
	//시작시 state생성
	GameStateClass = AFinalMinutesGameState::StaticClass();
}