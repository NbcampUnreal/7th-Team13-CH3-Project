#include "Framework/FinalMinutesGameState.h"
#include "Framework/FinalMinutesGameMode.h"
#include "Subsystems/SoundSubsystem.h"

AFinalMinutesGameState::AFinalMinutesGameState()
{
	KillCount = 0;
	//tick 돌아가게
	PrimaryActorTick.bCanEverTick = true;
	bIsGameOver = false;
}

void AFinalMinutesGameState::BeginPlay()
{
	Super::BeginPlay();
	
	KillCount = 0;
	OnKillCountChanged.Broadcast(KillCount);
	//테스트용 시간
	//GameTime = 590.0f;
	}

void AFinalMinutesGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	//게임이 시작되지 않았거나 이미 게임이 끝났다면 밑에 로직 무시.
	if (!bIsGameStarted || bIsGameCleared || bIsGameOver) return;
	
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
		return;
	}
	//현재 시간에 따른 새로운 페이즈 계산
	int32 TargetPhase = 0;
	//3분카레
	if (GameTime < 180.0f) TargetPhase = 1;
	//3분에서 7분
	else if (GameTime < 420.0f) TargetPhase = 2;
	//7분에서 10분
	else if (GameTime < 600.0f) TargetPhase = 3;

	//페이즈가 바뀌었을 때만 서브시스템 호출
	if (TargetPhase != CurrentBGMPhase)
	{
		CurrentBGMPhase = TargetPhase;
    
		USoundSubsystem* SoundSS = GetGameInstance()->GetSubsystem<USoundSubsystem>();
		if (SoundSS)
		{
			USoundBase* SelectedSound = nullptr;
			if(CurrentBGMPhase == 1) SelectedSound = BGM_Phase1;
			else if(CurrentBGMPhase == 2) SelectedSound = BGM_Phase2;
			else if(CurrentBGMPhase == 3) SelectedSound = BGM_Phase3;

			if (SelectedSound)
			{
				SoundSS->PlayBGMByPhase(SelectedSound);
			}
		}
	}
}

void AFinalMinutesGameState::AddKill()
{
	KillCount++;
	OnKillCountChanged.Broadcast(KillCount);
	//디버깅용 필수과제에 로그 찍히는거 있음 5.UI시스템 - 피해/킬로그
	UE_LOG(LogTemp, Warning, TEXT("킬 카운트 : %d"), KillCount);
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