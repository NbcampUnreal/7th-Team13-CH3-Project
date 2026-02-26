// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"

#include "ProjectileBullet.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class FINALMINUTES_API AProjectileBullet : public AActor
{
    GENERATED_BODY()

public:
    AProjectileBullet();

    /** 발사 시 무기(GA)에서 호출하여 데미지 스펙과 속도를 주입함 */
    virtual void InitializeProjectile(const FGameplayEffectSpecHandle& InSpecHandle, float InSpeed);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // 이전 프레임의 위치를 저장
    FVector LastLocation;

    // 디버그 라인 지속 시간
    UPROPERTY(EditDefaultsOnly, Category = "Debug")
    float DebugLineDuration = 10.0f;

    /** 실제 충돌 처리 */
    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                       FVector NormalImpulse, const FHitResult& Hit);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> CollisionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BulletMesh;

    /** 무기에서 생성된 데미지 정보 (충돌 시 적용할 내용) */
    FGameplayEffectSpecHandle DamageEffectSpecHandle;
};
