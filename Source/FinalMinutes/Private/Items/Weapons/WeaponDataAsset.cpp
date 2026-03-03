#include "FinalMinutes/Public/Items/Weapons/WeaponDataAsset.h"

UWeaponDataAsset::UWeaponDataAsset()
{
    // 특정 데이터 테이블 파일을 기본값으로 로드
    static ConstructorHelpers::FObjectFinder<UDataTable> DataTableAsset(TEXT("/Game/Data/DT_WeaponDataTable.DT_WeaponDataTable"));
    
    if (DataTableAsset.Succeeded())
    {
        WeaponSourceRow.DataTable = DataTableAsset.Object;
    }
}

void UWeaponDataAsset::RefreshDataFromTable()
{
    if (WeaponSourceRow.IsNull()) return;

    FWeaponData* RowData = WeaponSourceRow.GetRow<FWeaponData>(TEXT("ContextText"));
    if (RowData)
    {
        // 데이터 테이블의 내용을 DataAsset의 변수에 복사
        WeaponData = *RowData;
        
        // 에셋 저장 상태를 알림
        MarkPackageDirty();
        UE_LOG(LogTemp, Log, TEXT("[%s] Data refreshed from Table: %s"), *GetName(), *WeaponSourceRow.RowName.ToString());
    }
}

#if WITH_EDITOR
void UWeaponDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // WeaponSourceRow 속성이 변경되었을 때 자동으로 데이터 갱신
    if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UWeaponDataAsset, WeaponSourceRow))
    {
        RefreshDataFromTable();
    }
}
#endif