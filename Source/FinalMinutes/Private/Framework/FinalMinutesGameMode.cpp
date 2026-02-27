#include "Framework/FinalMinutesGameMode.h"
#include "Framework/FinalMinutesGameState.h"
#include "UI/FinalMinutesHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Subsystems/SaveSubsystem.h"
#include "Monster/SpawnVolume.h"
#include "Kismet/GameplayStatics.h"



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
	//ui만 조작 가능하게 마우스 소환
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this,0))
	{
		//마우스 커서 키고
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeUIOnly()); // UI만 클릭 가능하게
	}
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
	
	//게임 스테이트 불러와서 게임 시작하기
	AFinalMinutesGameState* GS = GetGameState<AFinalMinutesGameState>();
	if (GS)
	{
		//게임 시작하면 틱 돌기
		GS->bIsGameStarted = true;
	}
	
	//Start 버튼 누른 후 마우스 다시 커서 해제 에임모드 ON
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this,0))
	{
		PC->SetShowMouseCursor(false); //마우스 숨기기
		PC->SetInputMode(FInputModeGameOnly());//게임 조작만 가능하게
	}
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
	TArray<AActor*> SpawnVolumes;
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), SpawnVolumes);

	for (AActor* Actor : SpawnVolumes)
	{
		if (ASpawnVolume* Spawner = Cast<ASpawnVolume>(Actor))
		{
			GetWorldTimerManager().ClearTimer(Spawner->SpawnTimer);
		}
	}
	
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
	TArray<AActor*> SpawnVolumes;
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), SpawnVolumes);

	for (AActor* Actor : SpawnVolumes)
	{
		if (ASpawnVolume* Spawner = Cast<ASpawnVolume>(Actor))
		{
			GetWorldTimerManager().ClearTimer(Spawner->SpawnTimer);
		}
	}
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
	//게임 스테이트에 기록된 시간을 가져오기 킬 카운트도 같이
	AFinalMinutesGameState* GS = GetGameState<AFinalMinutesGameState>();
	float RealTime = GS ? GS->GameTime : 0.0f;
	int32 RealKill = GS ? GS->GetKillCount() : 0;
	
	if (auto* SaveSS = GetGameInstance()->GetSubsystem<USaveSubsystem>())
	{
		//현재 플레이 중이던 슬롯에 그대로 덮어 쓰기 하기
		//무조건 1번이 아니라 플레이 중이던 슬롯에 덮어쓰게하는거임
		SaveSS->SaveGameData(RealKill, RealTime, SaveSS->CurrentSlotName);
	}
	//게임 종료
	UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(
		this,
		0),
		EQuitPreference::Quit, false);
}

void AFinalMinutesGameMode::AdjustTimerAfterLoad(float LoadedTime)
{
	//전체 시간-플레이한 시간
	float RemainingTime = TimeLimit - LoadedTime;
	//남은시간 없으면 클리어 - 혹시나 클리어 이후에 저장된 파일이면.
	if (RemainingTime <= 0)
	{
		GameClear(); 
		return;
	}
	//기존에 돌아가던 타이머 관할 핸들러 취소시키기
	GetWorldTimerManager().ClearTimer(TimerHandle);
	//남은 시간만큼 예약해버리기 
	GetWorldTimerManager().SetTimer(TimerHandle,this,&AFinalMinutesGameMode::GameClear,RemainingTime,false);
}