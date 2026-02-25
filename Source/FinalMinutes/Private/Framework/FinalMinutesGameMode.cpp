#include "Framework/FinalMinutesGameMode.h"
#include "Framework/FinalMinutesGameState.h"
#include "UI/FinalMinutesHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Subsystems/SaveSubsystem.h"
#include "Monster/SpawnVolume.h"



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
	
	//게임 일시 정지 시 마우스 커서 나타나기
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this,0))
	{
		PC->SetShowMouseCursor(bIsPause); //정지면 켜고 아니면 끔
		//true 부르면 켜지고 false부르면 닫히게
		if (bIsPause)
		{
			//UI를 마우스로 클릭해야되서 모드 변경
			PC->SetInputMode(FInputModeGameAndUI());
		}
		else
		{
			//다시 게임으로 돌아가면 GameOnly로 모드 변경(시점 조작)
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
	//일시정시 함수
}

void AFinalMinutesGameMode::GameClear()
{
	//타이머 끝
	GetWorldTimerManager().ClearTimer(TimerHandle);
	//데이터 저장
	//SS = SaveSlot
	int32 RealKill = (GetGameState<AFinalMinutesGameState>()) ? GetGameState<AFinalMinutesGameState>()->GetKillCount() : 0;
	if (auto* SaveSS = GetGameInstance()->GetSubsystem<USaveSubsystem>())
	{
		SaveSS->SaveGameData(RealKill, TimeLimit, SaveSS->CurrentSlotName);
	}
	//몬스터 소환 끝
	
	//마우스 조작
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
	//클리어 UI 가져오기
}

void AFinalMinutesGameMode::GameOver()
{
	//타이머 끝
	GetWorldTimerManager().ClearTimer(TimerHandle);
	
	//데이터 저장
	int32 RealKill = (GetGameState<AFinalMinutesGameState>()) ? GetGameState<AFinalMinutesGameState>()->GetKillCount() : 0;
	float RealTime = GetWorldTimerManager().GetTimerElapsed(TimerHandle);

	if (auto* SaveSS = GetGameInstance()->GetSubsystem<USaveSubsystem>())
	{
		SaveSS->SaveGameData(RealKill, RealTime, SaveSS->CurrentSlotName);
	}
	//몬스터 소환 끝
	
	//마우스 조작
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
	//패배 UI 가져오기
	
}

void AFinalMinutesGameMode::GameExit()
{
	//게임 끄기 직전에 오토세이브 진행 하기
	//GameState가 없을 때 0 반환 하기
	int32 RealKill = (GetGameState<AFinalMinutesGameState>()) ? GetGameState<AFinalMinutesGameState>()->GetKillCount() : 0;
	float RealTime = GetWorldTimerManager().GetTimerElapsed(TimerHandle);
	
	if (auto* SaveSS = GetGameInstance()->GetSubsystem<USaveSubsystem>())
	{
		//현재 플레이 중이던 슬롯에 그대로 덮어 쓰기 하기
		//무조건 1번이 아니라 플레이 중이던 슬롯에 덮어쓰게하는거임
		SaveSS->SaveGameData(RealKill, RealTime, SaveSS->CurrentSlotName);
	}
	//게임 종료
	UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(this, 0), EQuitPreference::Quit, true);
}