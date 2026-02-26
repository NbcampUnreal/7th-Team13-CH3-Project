#include "Components/InventoryComponent.h"
#include "Character/Player/PlayerCharacter.h"
#include "Items/BaseItem.h"
#include "items/ItemData.h"
#include "Blueprint/UserWidget.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	Items.SetNum(InventorySize); // 인벤토리 크기를 InventorySize인 10만큼 생성합니다

	if (InventoryWidgetClass) // 우리가 UE Editor에서 InventoryWidgetClass를 선택해서 넣어주면 통과 합니다
	{
		// 이 인벤토리를 가지고 있는 플레이어 컨트롤러를 가져옵니다.
		APlayerController* PlayerController = Cast<APlayerController>(GetOwner()->GetInstigatorController());

		if (PlayerController) // 플레이어 컨트롤러가 정상적으로 가져와졌다면
		{
			// CreateWidget함수를 사용해서 UUserWidget을 만들어 줍니다. 첫 번째 매개변수로 이 위젯의 소유자 PlayerController를 넣어줍니다.
			// 가져온 PlayerController에 UI에를 뛰운다! 라고 생각하면 됩니다!
			InventoryWidget = CreateWidget<UUserWidget>(PlayerController, InventoryWidgetClass);
		}
	}
	
}

bool UInventoryComponent::AddItem(FName ItemID)
{
	if (ItemID == NAME_None)
        return false;

    // 0) MaxStack 읽기 (DT 없으면 기본 1)
    int32 MaxStack = 1;
    if (ItemDataTable)
    {
        static const FString Context(TEXT("AddItem"));
        if (FItemData* Row = ItemDataTable->FindRow<FItemData>(ItemID, Context))
        {
            MaxStack = FMath::Max(1, Row->MaxStack);
        }
    }

    int32 Remaining = 1; // 이번 호출에서 넣을 개수(현재는 1개)

    // 1) 스택 가능한 아이템(MaxStack > 1)일 때:
    //    기존 슬롯들 중 "아직 덜 찬 슬롯"을 먼저 채운다
    if (MaxStack > 1)
    {
        for (int32 i = 0; i < Items.Num() && Remaining > 0; ++i)
        {
            if (Items[i].ItemID == ItemID && Items[i].Quantity > 0 && Items[i].Quantity < MaxStack)
            {
                const int32 Space = MaxStack - Items[i].Quantity;
                const int32 ToAdd = FMath::Min(Space, Remaining);

                Items[i].Quantity += ToAdd;
                Remaining -= ToAdd;

                if (GEngine)
                {
                    const FString Msg = FString::Printf(
                        TEXT("스택 채움! [ %d번 ] 슬롯 [ %s ] 수량: %d/%d"),
                        i, *ItemID.ToString(), Items[i].Quantity, MaxStack
                    );
                    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, Msg);
                }
            }
        }
    }

    // 2) 남은 수량이 있으면 빈 슬롯에 새로 생성해서 넣는다
    while (Remaining > 0)
    {
        int32 EmptyIndex = INDEX_NONE;

        for (int32 i = 0; i < Items.Num(); ++i)
        {
            if (Items[i].ItemID == NAME_None || Items[i].Quantity <= 0)
            {
                EmptyIndex = i;
                break;
            }
        }

        // 빈 슬롯이 없으면 종료
        if (EmptyIndex == INDEX_NONE)
        {
            OnInventoryUpdated.Broadcast();
            return false;
        }

        // 새 스택 생성
        const int32 ToAdd = FMath::Min(MaxStack, Remaining);

        Items[EmptyIndex].ItemID = ItemID;
        Items[EmptyIndex].Quantity = ToAdd;
        Remaining -= ToAdd;

        if (GEngine)
        {
            const FString Msg = FString::Printf(
                TEXT("새 슬롯 생성! [ %d번 ] 슬롯에 [ %s ] 수량: %d/%d"),
                EmptyIndex, *ItemID.ToString(), Items[EmptyIndex].Quantity, MaxStack
            );
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, Msg);
        }
    }

    OnInventoryUpdated.Broadcast();
	return true;
}

void UInventoryComponent::UseItem(int32 SlotIndex)
{
	if (!Items.IsValidIndex(SlotIndex)) // 특정 슬롯에 아이템이 있는지 확인합니다.
	{
		// 없으면 조기리턴
		return;
	}

	FInventorySlot& Slot = Items[SlotIndex];
	if (Slot.ItemID == NAME_None || Slot.Quantity <= 0) // 아이템의 ID가 있는지 없는지 체크합니다.
	{
		return; // 없으면 조기리턴
	}

	if (Slot.ItemID == "Bullet")
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
		if (Player && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("총알 사용 안됨!"));
		}
	}
	else if (Slot.ItemID == "Gun")
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
		if (Player && GEngine)
		{
			// 총 장착한 후의 로직
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("총 장착"));
		}
	}
	
	else if (Slot.ItemID == "Health")
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
		if (Player && GEngine)
		{			
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("체력 회복!"));
			
			// 1개 사용
			Slot.Quantity -= 1;

			// 0개면 슬롯 비우기
			if (Slot.Quantity <= 0)
			{
				Slot.Quantity = 0;
				Slot.ItemID = NAME_None;
			}
		}
	}
	// 모든곳에 브로드캐스트합니다.
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::DropItem(int32 SlotIndex)
{
	// 해당 인덱스가 유효하지 않다면 리턴합니다.
	if (!Items.IsValidIndex(SlotIndex))
	{
		return;
	}

	FInventorySlot& Slot = Items[SlotIndex];
	if (Slot.ItemID == NAME_None || Slot.Quantity <= 0) // 버릴 아이템이 없으면 함수종료
	{
		return;
	}

	AActor* OwnerActor = GetOwner(); // 현재 오너와 아이템 테이블이 있는지 확인
	if (OwnerActor && ItemDataTable)
	{
		// 현재 액터의 앞 방향으로 1m 그리고 50cm쪽에 스폰할 위치를 설정
		FVector SpawnLocation = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 100.0f +
			FVector(0, 0, 50.0f);
		// 현재 액터의 방향은 그대로 가져옵니다.
		FRotator SpawnRotation = OwnerActor->GetActorRotation();
		// SpawnParams - 액터 생성시 필요한 옵션 변수
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerActor; // 누가 소유했는지 알려줍니다.
		SpawnParams.Instigator = OwnerActor->GetInstigator(); // 누가 이아이템을 제공했는지 알려줍니다.

		// 아이템 테이블에서 떨어뜨릴 아이템을 찾습니다.
		FItemData* Row = ItemDataTable->FindRow<FItemData>(Slot.ItemID, "");
		if (Row && Row->ItemActorClass) // 아이템과 아이템의 설계도가 있다면
		{
			// 실제로 현재 월드에 아이템을 소환합니다.
			GetWorld()->SpawnActor<ABaseItem>(Row->ItemActorClass, SpawnLocation, SpawnRotation, SpawnParams);
		}
	}
	
	// 1개만 버리기
	Slot.Quantity -= 1;

	// 0개면 슬롯 비우기
	if (Slot.Quantity <= 0)
	{
		Slot.Quantity = 0;
		Slot.ItemID = NAME_None;
	}
	
	// 인벤토리UI를 업데이트 시킵니다.
	OnInventoryUpdated.Broadcast();

	if (GEngine)
	{
		// 아이템 버린것을 출력합니다.
		FString Msg = FString::Printf(TEXT("[ %d번 ] 슬롯의 아이템을 버렸습니다."), SlotIndex);
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, Msg);
	}
}
