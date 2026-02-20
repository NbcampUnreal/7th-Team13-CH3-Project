#include "Framework/FinalMinutesGameMode.h"
#include "Framework/FinalMinutesGameState.h"
#include "UI/FinalMinutesHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


AFinalMinutesGameMode::AFinalMinutesGameMode()
{
	// 게임스테이트 클래스 등록 AddKill
	GameStateClass = AFinalMinutesGameState::StaticClass();
	//HUD 클래스 리스너
	HUDClass = AFinalMinutesHUD::StaticClass();
}

void AFinalMinutesGameMode::BeginPlay()
{
	Super::BeginPlay();
	GameStart();
}

void AFinalMinutesGameMode::GameStart()
{
	//게임 클리어 함수 호출 예약
	// 타이머핸들, 델리게이트(함수), 실행 간격, 반복 여부
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		this, 
		&AFinalMinutesGameMode::GameClear, 
		TimeLimit, 
		false);
}

void AFinalMinutesGameMode::GamePause(bool bIsPause)
{
	//언리얼 엔진 게임 시간 타임 스톱!!
	UGameplayStatics::SetGamePaused(GetWorld(), bIsPause);
	
	//true 부르면 켜지고 false부르면 닫히게
	if (bIsPause)
	{
		//Pause 메뉴 띄우기
	}
	else
	{
		// 메뉴 닫기 
	}
}

void AFinalMinutesGameMode::GameClear()
{
	//타이머 끝
	GetWorldTimerManager().ClearTimer(TimerHandle);
	//몬스터 소환 끝
	
	//클리어 UI 가져오기
	
}

void AFinalMinutesGameMode::GameOver()
{
	//타이머 끝
	GetWorldTimerManager().ClearTimer(TimerHandle);
	//몬스터 소환 끝
	
	//패배 UI 가져오기
	
}

void AFinalMinutesGameMode::GameExit()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
}