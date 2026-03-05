#include "AbilitySystem/AnimNotify/AN_MFootStep.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UAN_MFootStep::UAN_MFootStep()
{
    // GameplayCue 태그설정
    FootStepCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Monster.Footstep"));
    // 바닥까지 탐색 거리
    TraceDistance = 100.f;
}

void UAN_MFootStep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    
    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;
    
    FHitResult HitResult;
    // Mesh아래쪽으로 100만큼 라인트레이스 탐색
    FVector Start = MeshComp->GetComponentLocation();
    FVector End = Start - FVector(0.f, 0.f, TraceDistance);

    // 자기 자신은 안맞게 Ignore설정
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);
    // 맞은 물체의 물리재질정보를 가져오겠다 true
    QueryParams.bReturnPhysicalMaterial = true;
    
    // 라인트레이스
    bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
        HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams
    );

    if (bHit && HitResult.PhysMaterial.IsValid())
    {
        UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
        if (!ASC) return;
        
        FGameplayCueParameters Params;
        // 맞은 위치
        Params.Location = HitResult.Location;
        // 파티클 추가할때 사용
        Params.Normal = HitResult.ImpactNormal;
        // 맞은 재질
        Params.PhysicalMaterial = HitResult.PhysMaterial;
        // 소리낸 주체
        Params.EffectCauser = Owner;
        
        // BP_GCN_FootStep에서 재질에 따라 설정된 소리 틀어줌
        ASC->ExecuteGameplayCue(FootStepCueTag, Params);
    }
}