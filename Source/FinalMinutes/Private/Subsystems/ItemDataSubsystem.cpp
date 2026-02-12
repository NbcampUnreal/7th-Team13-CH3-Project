#include "FinalMinutes/Public/Subsystems/ItemDataSubsystem.h"

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