// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CharacterPhysicalMaterial.generated.h"

/**
 * UMyPhysicalMaterial
 * 각 신체 부위(Physics Asset의 Body)에 할당되어 해당 부위가 어떤 논리적 위치(태그)인지 정의합니다.
 */
UCLASS()
class FINALMINUTES_API UCharacterPhysicalMaterial : public UPhysicalMaterial
{
    GENERATED_BODY()

public:
    UCharacterPhysicalMaterial();

    /** * 이 부위에 해당하는 GameplayTag입니다. (예: Damage.HitRegion.Head, Damage.HitRegion.Leg_L) 
     * 에디터에서 Physical Material 에셋을 생성한 후 직접 지정합니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (Categories = "Damage.HitRegion"))
    FGameplayTag HitRegionTag;
};
