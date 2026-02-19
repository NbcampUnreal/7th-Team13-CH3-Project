#include "Framework/FinalMinutesGameMode.h"
#include "Framework/FinalMinutesGameState.h"
#include "UI/FinalMinutesHUD.h"

AFinalMinutesGameMode::AFinalMinutesGameMode()
{
	// 게임스테이트 클래스 등록 AddKill
	GameStateClass = AFinalMinutesGameState::StaticClass();
	//HUD 클래스 리스너
	HUDClass = AFinalMinutesHUD::StaticClass();
}
