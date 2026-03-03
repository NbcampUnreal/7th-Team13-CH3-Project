#include "Monster/Notify/UAN_DealDamage.h"
#include"AbilitySystemComponent.h"
#include"AbilitySystemInterface.h"
#include"Kismet/KismetSystemLibrary.h"

UAN_DealDamage::UAN_DealDamage()
{
    // 기본값
    TraceRadius = 50.0f;
    TraceStartDistance = 50.0f;
    TraceEndDistance = 150.0f;
    bShowDebug = false;
}

void UAN_DealDamage::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner)
    {
        return;
    }

    // 캐릭터의 전방 방향 계산
    FVector Forward = Owner->GetActorForwardVector();
    FVector Start = Owner->GetActorLocation() + Forward * TraceStartDistance;
    FVector End = Owner->GetActorLocation() + Forward * TraceEndDistance;

    // Sphere Trace 실행
    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(Owner);

    EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

    bool bHit = UKismetSystemLibrary::SphereTraceMulti(
        Owner->GetWorld(),
        Start,
        End,
        TraceRadius,
        UEngineTypes::ConvertToTraceType(ECC_Pawn),
        false,                    // bTraceComplex
        ActorsToIgnore,
        DebugType,
        HitResults,
        true,                     // bIgnoreSelf
        FLinearColor::Red,        // TraceColor
        FLinearColor::Green,      // TraceHitColor
        2.0f                      // DrawTime
    );

    if (!bHit)
    {
        return;
    }

    // 공격자의 ASC 가져오기
    IAbilitySystemInterface* AttackerASI = Cast<IAbilitySystemInterface>(Owner);
    if (!AttackerASI)
    {
        return;
    }

    UAbilitySystemComponent* AttackerASC = AttackerASI->GetAbilitySystemComponent();
    if (!AttackerASC || !DamageEffectClass)
    {
        return;
    }

    // 맞은 모든 액터에 Effect 적용
    TSet<AActor*> ProcessedActors; // 중복 방지

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || ProcessedActors.Contains(HitActor))
        {
            continue;
        }

        ProcessedActors.Add(HitActor);

        // 맞은 대상의 ASC 가져오기
        IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HitActor);
        if (!TargetASI)
        {
            continue;
        }

        UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
        if (!TargetASC)
        {
            continue;
        }

        // Effect 적용
        FGameplayEffectContextHandle Context = AttackerASC->MakeEffectContext();
        Context.AddSourceObject(Owner);

        FGameplayEffectSpecHandle SpecHandle = AttackerASC->MakeOutgoingSpec(
            DamageEffectClass, 1.0f, Context);

        if (SpecHandle.IsValid())
        {
            AttackerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

            UE_LOG(LogTemp, Log, TEXT("Damage applied to:%s"), *HitActor->GetName());
        }
    }
}
