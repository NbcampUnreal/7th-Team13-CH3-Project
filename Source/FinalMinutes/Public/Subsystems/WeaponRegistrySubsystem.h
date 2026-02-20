// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "WeaponRegistrySubsystem.generated.h"

class UWeaponDataAsset;
/**
 * 
 */
UCLASS()
class FINALMINUTES_API UWeaponRegistrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/** 태그를 통해 무기 데이터 에셋을 검색 (이미 로드되어 있다고 가정하거나 로드 요청) */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    UWeaponDataAsset* GetWeaponDataByTag(FGameplayTag InTag);
	
private:
    /** 메모리 효율을 위해 태그와 에셋 포인터를 맵핑하여 캐싱 */
    UPROPERTY()
    TMap<FGameplayTag, TObjectPtr<UWeaponDataAsset>> WeaponDataCache;
};
