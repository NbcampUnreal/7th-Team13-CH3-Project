#pragma once

#include "CoreMinimal.h"
#include "FWeaponData.h"
#include "Engine/DataAsset.h"
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
    UWeaponDataAsset();
    
    // 1. 데이터 테이블과 행을 선택하는 핸들
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data Source")
    FDataTableRowHandle WeaponSourceRow;

    // 2. 실제 데이터 (테이블에서 불러온 값으로 채워짐)
    // EditDefaultsOnly 대신 VisibleAnywhere를 써서 수동 수정을 막고 테이블 동기화를 유도할 수 있습니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data")
    FWeaponData WeaponData;

    /** 테이블로부터 데이터를 최신화하는 함수 */
    UFUNCTION(CallInEditor, Category = "Data Source")
    void RefreshDataFromTable();

#if WITH_EDITOR
    // 에디터에서 값이 바뀌면 자동으로 테이블 데이터를 다시 불러옴
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    /** Asset Manager에서 이 에셋을 고유하게 식별하기 위한 ID 반환 */
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(FName("WeaponData"), GetFName());
    }
};
