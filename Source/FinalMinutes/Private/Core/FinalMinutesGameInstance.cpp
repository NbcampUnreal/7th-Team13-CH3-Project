#include "Core/FinalMinutesGameInstance.h"
#include "Subsystems/ItemDataSubsystem.h"
#include "Subsystems/SoundSubsystem.h"
#include "Subsystems/SaveSubsystem.h"

void UFinalMinutesGameInstance::Init()
{
	Super::Init();
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