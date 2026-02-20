#pragma once

#include "CoreMinimal.h"
#include "FWeaponData.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "WeaponDataAsset.generated.h"


/**
 * 무기 제작을 위한 정적 설계도 클래스
 * UPrimaryDataAsset을 상속받아 AssetManager의 비동기 로딩 기능을 활용합니다.
 */
UCLASS(BlueprintType)
class FINALMINUTES_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// 데이터 테이블의 행 정보를 그대로 들고 있거나, 개별 프로퍼티로 보유
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	FWeaponData WeaponData;
	
    /** Asset Manager에서 이 에셋을 고유하게 식별하기 위한 ID 반환 */
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(FName("WeaponData"), GetFName());
    }
};
