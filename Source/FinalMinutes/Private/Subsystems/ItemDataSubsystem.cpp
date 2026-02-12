#include "FinalMinutes/Public/Subsystems/ItemDataSubsystem.h"

UItemDataSubsystem::UItemDataSubsystem()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> 
	DT_WeaponDataTable
	(TEXT("/Game/Data/DT_WeaponDataTable.DT_WeaponDataTable"));
	
	if (DT_WeaponDataTable.Succeeded())
	{
		WeaponDataTable = DT_WeaponDataTable.Object;
		//디버깅용
		UE_LOG(LogTemp, Warning, TEXT("DT_WeaponDataTable을 찾았다."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DT_WeaponDataTable을 찾지 못했습니다."));
	}
}


FWeaponData UItemDataSubsystem::GetWeaponData(FGameplayTag Tag)
{
	if (!WeaponDataTable)
	{
		return FWeaponData();
	}
	TArray<FWeaponData*> AllRows;
	WeaponDataTable->GetAllRows<FWeaponData>(TEXT(""), AllRows);
	
	for (auto Row : AllRows) 
	{
		if (Row->WeaponTag.MatchesTagExact(Tag)) return *Row;
	}
	return FWeaponData();
}

FMonsterData UItemDataSubsystem::GetMonsterData(FGameplayTag Tag)
{
	if (!MonsterDataTable)
	{
		return FMonsterData();
	}
	TArray<FMonsterData*> AllRows;
	MonsterDataTable->GetAllRows<FMonsterData>(TEXT(""), AllRows);
	
	for (auto Row : AllRows) 
	{
		if (Row->MonsterTag.MatchesTagExact(Tag)) return *Row;
	}
	return FMonsterData();
}