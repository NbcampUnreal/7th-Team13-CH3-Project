#include "AbilitySystem/Abilities/GA_Interact.h"
#include "Character/Player/PlayerCharacter.h"
#include "Components/InventoryComponent.h"
#include "Items/BaseItem.h"

UGA_Interact::UGA_Interact()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;


    InteractDistance = 600.0f;
}

void UGA_Interact::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
    if (!Avatar)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UInventoryComponent* InventoryComponent = Avatar->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryComponent를 Avatar에서 찾지 못했습니다."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    APlayerCharacter* Player = Cast<APlayerCharacter>(Avatar);
    if (!Player)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ✅ 캐릭터가 Tick/Timer로 미리 잡아둔 '바라보는 아이템'
    ABaseItem* HitItem = Player->GetFocusedItem();
    if (!HitItem)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    //  드랍 아이템이면 OverridePickupAmount(예: 1, 5, 10)를 쓰고,
    //  아니면 DT PickupAmount를 쓰게 -1 전달
    const int32 Amount = (HitItem->OverridePickupAmount > 0) ? HitItem->OverridePickupAmount : -1;

    const bool bAdded = InventoryComponent->AddItem(HitItem->ItemID, Amount);

    if (bAdded)
    {
        HitItem->Destroy();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("인벤토리가 가득 찼습니다!"));
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}