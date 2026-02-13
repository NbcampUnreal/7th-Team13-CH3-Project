#include "FinalMinutes/Public/UI/FinalMinutesHUD.h"
#include "FinalMinutes/Public/Framework/FinalMinutesGameState.h"

void AFinalMinutesHUD::BeginPlay()
{
	Super::BeginPlay();
	
	AFinalMinutesGameState* GS = GetWorld()->GetGameState<AFinalMinutesGameState>();
	
	if (GS)// 게임 스테이트라면
	{
		GS->OnKillCountChanged.AddDynamic(this, &AFinalMinutesHUD::OnKillCountUpdated);
		//디버깅
		UE_LOG(LogTemp, Warning, TEXT("끼얏호우 HUD 킬카운트"))
	}
}

void AFinalMinutesHUD::OnKillCountUpdated(int32 NewKillCount)
{
	UE_LOG(LogTemp, Warning, TEXT("킬 달성 메세지 : %d"),NewKillCount);
}