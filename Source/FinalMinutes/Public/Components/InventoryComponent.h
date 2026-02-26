#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

USTRUCT(BlueprintType)
struct FInventorySlot // 아이템슬롯 == 아이템 한칸에 들어갈 정보들
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Slot")
	FName ItemID; // 아이템 DT_ItemData를 찾아갈 ID입니다
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory Slot")
	int32 Quantity = 0; // 스택 수량
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FINALMINUTES_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventorySlot> Items; // 아이템슬롯을 TArray배열로 만들어서 여러 슬롯을 가진 아이템 배열 완성

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 InventorySize = 10; // 인벤토리 크기 10칸 입니다

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|UI")
	TSubclassOf<UUserWidget> InventoryWidgetClass; // 인벤토리를 시각화 하기위해서 어떤 위젯을 사용할지 담는 변수

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI")
	UUserWidget* InventoryWidget; // 위에서 위젯을 선택하고 생성한 위젯을 담는 변수

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Data")
	UDataTable* ItemDataTable; // 우리가 아까 만든 데이터 테이블을 저장할 변수
		
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemID); // 아이템 줍는 함수, 위젯 블루프린트에서 호출(BlueprintCallable)
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DropItem(int32 SlotIndex);	
};
