// WeaponRegistrySubsystem.cpp

#include "Subsystems/WeaponRegistrySubsystem.h"
#include "Engine/AssetManager.h"
#include "Items/Weapons/WeaponDataAsset.h"

void UWeaponRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    // 부모 클래스의 Initialize를 반드시 먼저 호출해야 합니다.
    Super::Initialize(Collection);

    UAssetManager& AssetManager = UAssetManager::Get();
    TArray<FPrimaryAssetId> AssetIds;

    // 프로젝트 설정(Asset Manager)에 등록한 "WeaponData" 타입의 모든 ID를 가져옵니다.
    AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("WeaponData"), AssetIds);
    
    for (const FPrimaryAssetId& Id : AssetIds)
    {
        // 에셋 경로를 찾아 동기식으로 로드합니다.
        // 데이터 에셋은 용량이 작아 게임 시작 시 한 번에 로드해도 무리가 없습니다.
        UObject* LoadedAsset = AssetManager.GetStreamableManager().LoadSynchronous(AssetManager.GetPrimaryAssetPath(Id));
        UWeaponDataAsset* WeaponAsset = Cast<UWeaponDataAsset>(LoadedAsset);

        if (WeaponAsset)
        {
            FGameplayTag Tag = WeaponAsset->WeaponData.WeaponTag; // DataAsset 내부의 태그 변수명에 맞게 수정하세요.
            if (Tag.IsValid())
            {
                WeaponDataCache.Add(Tag, WeaponAsset);
            }
            
        }
    }
}

UWeaponDataAsset* UWeaponRegistrySubsystem::GetWeaponDataByTag(FGameplayTag InTag)
{
    if (!InTag.IsValid()) return nullptr;

    // Initialize에서 이미 모든 데이터를 로드했으므로 맵에서 찾기만 하면 됩니다.
    if (WeaponDataCache.Contains(InTag))
    {
        return WeaponDataCache[InTag];
    }
    return nullptr;
}