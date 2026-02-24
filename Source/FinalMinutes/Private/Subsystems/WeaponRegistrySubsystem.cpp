// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/WeaponRegistrySubsystem.h"
#include "Engine/AssetManager.h"
#include "Items/Weapons/WeaponDataAsset.h"

UWeaponDataAsset* UWeaponRegistrySubsystem::GetWeaponDataByTag(FGameplayTag InTag)
{
    if (!InTag.IsValid()) return nullptr;

    // 1. 캐시 확인 (런타임 성능 최적화)
    if (WeaponDataCache.Contains(InTag))
    {
        return WeaponDataCache[InTag];
    }

    // 2. UAssetManager 싱글톤 참조
    UAssetManager& AssetManager = UAssetManager::Get();

    TArray<FPrimaryAssetId> AssetIds;
    // "WeaponData"는 프로젝트 설정의 Primary Asset Types에 등록된 이름이어야 함
    AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("WeaponData"), AssetIds);

    for (const FPrimaryAssetId& Id : AssetIds)
    {
        // 로드된 에셋 객체 획득 (로드되지 않았다면 nullptr 반환)
        UObject* AssetObj = AssetManager.GetPrimaryAssetObject(Id);
        UWeaponDataAsset* WeaponAsset = Cast<UWeaponDataAsset>(AssetObj);

        if (WeaponAsset && WeaponAsset->WeaponData.WeaponTag == InTag)
        {
            WeaponDataCache.Add(InTag, WeaponAsset);
            return WeaponAsset;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("WeaponRegistry: Could not find DataAsset for Tag: %s"), *InTag.ToString());
    return nullptr;
}
