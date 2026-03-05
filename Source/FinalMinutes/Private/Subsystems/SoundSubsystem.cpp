#include "FinalMinutes/Public/Subsystems/SoundSubsystem.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void USoundSubsystem::PlayBGMByPhase(USoundBase* NewSound)
{
	if (!NewSound) return;
	
	//이미 음악이 나오고 있다면 끄기
	if (BackgroundMusicComponent && BackgroundMusicComponent->IsPlaying())
	{
		BackgroundMusicComponent->Stop();
	}
	
	//새 음액 재생
	BackgroundMusicComponent = UGameplayStatics::SpawnSound2D(GetWorld(), NewSound);
}
