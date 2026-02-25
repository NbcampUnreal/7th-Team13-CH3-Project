// WeaponRegistrySubsystem.cpp

#include "Subsystems/WeaponRegistrySubsystem.h"
#include "Engine/AssetManager.h"
#include "Items/Weapons/WeaponDataAsset.h"

void UWeaponRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    UE_LOG(LogTemp, Warning, TEXT("WeaponRegistry: Initialize"));
    // 부모 클래스의 Initialize를 반드시 먼저 호출해야 합니다.
    Super::Initialize(Collection);

    UAssetManager& AssetManager = UAssetManager::Get();
    TArray<FPrimaryAssetId> AssetIds;

    // 프로젝트 설정(Asset Manager)에 등록한 "WeaponData" 타입의 모든 ID를 가져옵니다.
    AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("WeaponData"), AssetIds);
    
    if (AssetIds.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Asset을 불러오지 못했습니다."));
    } else
    {
        UE_LOG(LogTemp, Warning, TEXT("Asset을 불러왔습니다"));
    }

    UE_LOG(LogTemp, Warning, TEXT("for문 진입 전"));
    for (const FPrimaryAssetId& Id : AssetIds)
    {
        UE_LOG(LogTemp, Warning, TEXT("for문 진입 후"));
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
                // 로그를 남겨 로드가 잘 되었는지 확인할 수 있습니다.
                UE_LOG(LogTemp, Log, TEXT("WeaponRegistry: Successfully loaded [%s] with Tag [%s]"), *WeaponAsset->GetName(), *Tag.ToString());
            }
            
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("WeaponAsset을 불러오지 못했습니다"));
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

    UE_LOG(LogTemp, Warning, TEXT("WeaponRegistry: Could not find WeaponData for Tag: %s"), *InTag.ToString());
    return nullptr;
}