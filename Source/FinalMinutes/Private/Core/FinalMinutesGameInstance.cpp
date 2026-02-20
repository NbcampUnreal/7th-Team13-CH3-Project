#include "Core/FinalMinutesGameInstance.h"
#include "Subsystems/ItemDataSubsystem.h"
#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/SaveSubsystem.h"

void UFinalMinutesGameInstance::Init()
{
	Super::Init();
	//디버깅용 
	UE_LOG(LogTemp, Warning, TEXT("Init 저희가 참말로 조아하져잉 잘 작동 됩니다"));
}	

UItemDataSubsystem* UFinalMinutesGameInstance::GetItemSubsystem() const
{
	return GetSubsystem<UItemDataSubsystem>();
}

USoundSubsystem* UFinalMinutesGameInstance::GetSoundSubsystem() const
{
	return GetSubsystem<USoundSubsystem>();
}

USaveSubsystem* UFinalMinutesGameInstance::GetSaveSubsystem() const
{
	return GetSubsystem<USaveSubsystem>();
}