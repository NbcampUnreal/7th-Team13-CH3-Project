#include "FinalMinutes/Public/Subsystems/SoundSubsystem.h"
#include "GameplayTagContainer.h"

void USoundSubsystem::PlaySoundTag(FGameplayTag SoundTag, FVector Location)
{
	//디버깅용
	UE_LOG(LogTemp, Warning, TEXT("여따가 소리가 나옵니다잉: %s"), *Location.ToString());
	//어따 소리가 나옵니다잉 : X..Y..Z..
	
	//태그 확인
	//if(SoundTag.MatchesTag(FGameplayTag::RequestGameplayTag("Sound.Weapon.Fire")))
	//UGameplayStatic::PlaySoundAtLocation(여기에 뭘넣어야하지) - > 태그가 발사라면 수행
	
}

void USoundSubsystem::PlayBGM(FGameplayTag BGMTag)
{
	//디버깅용
	UE_LOG(LogTemp, Warning, TEXT("워후!브금이당!: [%s]"),*BGMTag.ToString());
}
