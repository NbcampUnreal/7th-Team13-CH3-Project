#include "AbilitySystem/Abilities/GA_Interact.h"
#include "Character/Player/PlayerCharacter.h"
#include "Components/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Engine/Engine.h"

UGA_Interact::UGA_Interact()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Interact")));
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

    const bool bAdded = InventoryComponent->AddItem(HitItem->ItemID);

    if (bAdded)
    {
        HitItem->Destroy();
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("인벤토리가 가득 찼습니다!"));
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}