#include "AbilitySystem/Abilities/GA_Interact.h"
#include "Character/Player/PlayerCharacter.h"
#include "Components/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "DrawDebugHelpers.h" // 디버그 선(레이저)을 눈으로 보기 위해 필요함

UGA_Interact::UGA_Interact()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Interact")));
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
    
    // ✅ 인벤토리 컴포넌트 가져오기
    AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
    if (!Avatar)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ✅ 인벤토리 컴포넌트 가져오기
    UInventoryComponent* InventoryComponent = Avatar->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryComponent를 Avatar에서 찾지 못했습니다."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    APlayerController* PC = ActorInfo->PlayerController.Get();
    if (!PC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
    FVector StartTrace = CameraLocation;
    FVector EndTrace = StartTrace + (CameraRotation.Vector() * InteractDistance);
    
    FHitResult HitResult;
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ActorInfo->AvatarActor.Get());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartTrace,
        EndTrace,
        ECC_Visibility,
        QueryParams
    );
    
    FColor DebugColor = bHit ? FColor::Green : FColor::Red;
    DrawDebugLine(GetWorld(), StartTrace, EndTrace, DebugColor, false, 2.0f, 0, 2.0f);
    
    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();
        ABaseItem* HitItem = Cast<ABaseItem>(HitActor);
        
        // 상호작용 대상 로그 찍어보기
        UE_LOG(LogTemp, Warning, TEXT("상호작용 대상 이름: %s"), *HitActor->GetName());

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
    }
    
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
