#include "Components/InventoryComponent.h"
#include "Character/Player/PlayerCharacter.h"
#include "Items/BaseItem.h"
#include "items/ItemData.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "Character/Components/CombatComponent.h"

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
    return AddItem(ItemID, -1); // 기본 줍기 = DT PickupAmount
}

bool UInventoryComponent::AddItem(FName ItemID, int32 Amount)
{
    if (ItemID == NAME_None) return false;

    // 기본값: 무한 스택 + 1개 줍기
    int32 MaxStack = 0;
    int32 PickupAmount = 1;

    if (ItemDataTable)
    {
        static const FString Context(TEXT("AddItem"));
        if (FItemData* Row = ItemDataTable->FindRow<FItemData>(ItemID, Context))
        {
            MaxStack = Row->MaxStack;
            PickupAmount = FMath::Max(1, Row->PickupAmount);
        }
    }

    // Amount가 들어오면 그걸 쓰고, 아니면 DT PickupAmount 사용
    int32 Remaining = (Amount > 0) ? Amount : PickupAmount;

    const bool bInfiniteStack = (MaxStack <= 0);

    // 1) 무한 스택: 같은 ItemID 슬롯 하나에 전부 누적
    if (bInfiniteStack)
    {
        for (int32 i = 0; i < Items.Num(); ++i)
        {
            if (Items[i].ItemID == ItemID && Items[i].Quantity > 0)
            {
                Items[i].Quantity += Remaining;
                Remaining = 0;
                break;
            }
        }

        if (Remaining > 0)
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

            if (EmptyIndex == INDEX_NONE)
            {
                OnInventoryUpdated.Broadcast();
                return false;
            }

            Items[EmptyIndex].ItemID = ItemID;
            Items[EmptyIndex].Quantity = Remaining;
            Remaining = 0;
        }

        OnInventoryUpdated.Broadcast();
        return true;
    }

    // 유한 스택(= MaxStack > 0)
    MaxStack = FMath::Max(1, MaxStack);

    // 기존 덜 찬 스택 채우기
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
            }
        }
    }

    // 남은 수량은 새 슬롯 생성
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

        if (EmptyIndex == INDEX_NONE)
        {
            OnInventoryUpdated.Broadcast();
            return false;
        }

        const int32 ToAdd = FMath::Min(MaxStack, Remaining);
        Items[EmptyIndex].ItemID = ItemID;
        Items[EmptyIndex].Quantity = ToAdd;
        Remaining -= ToAdd;
    }

	UE_LOG(LogTemp, Warning, TEXT("[INV] AddItem %s qty=%d role=%d"),
	*ItemID.ToString(), GetItemQuantity(ItemID), (int32)GetOwner()->GetLocalRole());
	
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
    
    if (Slot.ItemID == "Weapon_rifle" || Slot.ItemID == "Weapon_shotgun")
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
	    if (!Player) return;

	    UAbilitySystemComponent* ASC = Player->FindComponentByClass<UAbilitySystemComponent>();
	    if (!ASC) return;
	    
	    const float CurHealth = ASC->GetNumericAttribute(UCharacterAttributeSet::GetHealthAttribute());
	    const float MaxHealth = ASC->GetNumericAttribute(UCharacterAttributeSet::GetMaxHealthAttribute());
	    
		// 체력이 이미 꽉 찼으면 사용 불가(아이템 소모도 X)
		if (CurHealth >= MaxHealth - KINDA_SMALL_NUMBER)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("체력이 이미 가득 찼습니다!"));
			}
			return;
		}
		
		const float HealAmount = 10.f;
		const float NewHealth = FMath::Clamp(CurHealth + HealAmount, 0.f, MaxHealth);

		ASC->SetNumericAttributeBase(UCharacterAttributeSet::GetHealthAttribute(), NewHealth);
		
		Slot.Quantity -= 1;
		if (Slot.Quantity <= 0)
		{
			Slot.Quantity = 0;
			Slot.ItemID = NAME_None;
		}
	}
	// 모든곳에 브로드캐스트합니다.
	OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::DropItem(int32 SlotIndex, int32 DropRequest)
{
    if (!Items.IsValidIndex(SlotIndex))
        return false;

    FInventorySlot& Slot = Items[SlotIndex];
    if (Slot.ItemID == NAME_None || Slot.Quantity <= 0)
        return false;

    DropRequest = FMath::Max(1, DropRequest);

    // 실제로 버릴 수 있는 수량(남은 것보다 많이 못 버림)
    int32 ActuallyDrop = FMath::Min(DropRequest, Slot.Quantity);

    // 아이템 스폰
    AActor* OwnerActor = GetOwner();
    if (OwnerActor && ItemDataTable)
    {
        FVector SpawnLocation =
            OwnerActor->GetActorLocation()
            + OwnerActor->GetActorForwardVector() * 120.0f
            + FVector(0, 0, 80.0f);

        FRotator SpawnRotation = FRotator::ZeroRotator;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = OwnerActor;
        SpawnParams.Instigator = OwnerActor->GetInstigator();
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        FItemData* Row = ItemDataTable->FindRow<FItemData>(Slot.ItemID, TEXT("DropItem"));
        if (Row && Row->ItemActorClass)
        {
            ABaseItem* Dropped = GetWorld()->SpawnActor<ABaseItem>(
                Row->ItemActorClass, SpawnLocation, SpawnRotation, SpawnParams);

            if (Dropped)
            {
                Dropped->ItemID = Slot.ItemID;
            	// 드랍된 아이템은 다시 주울 때 "실제 버린 만큼"만 줍게
                Dropped->OverridePickupAmount = ActuallyDrop;
            }
        }
    }

    // 인벤 수량 감소
    Slot.Quantity -= ActuallyDrop;
    if (Slot.Quantity <= 0)
    {
        Slot.Quantity = 0;
        Slot.ItemID = NAME_None;
    }

    OnInventoryUpdated.Broadcast();

    if (GEngine)
    {
        const FString Msg = FString::Printf(
            TEXT("[ %d번 ] 슬롯에서 %s 를 %d개 버렸습니다."),
            SlotIndex, *Slot.ItemID.ToString(), ActuallyDrop
        );
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, Msg);
    }

    return true;
}

int32 UInventoryComponent::GetItemQuantity(FName InItemID) const
{
	if (InItemID.IsNone()) return 0;

	int32 Total = 0;
	for (const FInventorySlot& Slot : Items)
	{
		if (Slot.ItemID == InItemID)
		{
			Total += Slot.Quantity;
		}
	}
	return Total;
}

bool UInventoryComponent::ConsumeItem(FName InItemID, int32 Amount)
{
	if (InItemID.IsNone() || Amount <= 0) return false;

	// 먼저 충분한지 체크(여러 슬롯에 나뉘어 있을 수도 있으니까 Total로)
	const int32 Have = GetItemQuantity(InItemID);
	if (Have < Amount) return false;

	int32 Remaining = Amount;

	// 슬롯을 돌면서 차감
	for (int32 i = 0; i < Items.Num() && Remaining > 0; ++i)
	{
		FInventorySlot& Slot = Items[i];
		if (Slot.ItemID != InItemID) continue;
		if (Slot.Quantity <= 0) continue;

		const int32 Used = FMath::Min(Slot.Quantity, Remaining);
		Slot.Quantity -= Used;
		Remaining -= Used;

		// 슬롯 정리(0이면 비움)
		if (Slot.Quantity <= 0)
		{
			Slot.Quantity = 0;
			Slot.ItemID = NAME_None;
		}
	}

	OnInventoryUpdated.Broadcast();
	return true;
}
