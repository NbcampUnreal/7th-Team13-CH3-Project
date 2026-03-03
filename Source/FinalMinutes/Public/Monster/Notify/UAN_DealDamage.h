#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "UAN_DealDamage.generated.h"

class UGameplayEffect;

UCLASS()
class FINALMINUTES_API UAN_DealDamage : public UAnimNotify
{
    GENERATED_BODY()
    
public:
    UAN_DealDamage();
    
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                        const FAnimNotifyEventReference& EventReference) override;
    
    /** Sphere Trace 반경 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float TraceRadius = 50.0f;

    /** Trace 시작 거리 (캐릭터 전방) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float TraceStartDistance = 50.0f;

    /** Trace 종료 거리 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float TraceEndDistance = 150.0f;

    /** 적용할 Damage Effect 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    /** 디버그 드로잉 표시 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebug = true;
};
