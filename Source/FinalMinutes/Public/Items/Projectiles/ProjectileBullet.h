// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h" // GAS 스펙 핸들 사용을 위해 필요
#include "ProjectileBullet.generated.h"

// 전방 선언: 컴파일 속도 향상 및 의존성 감소
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

/**
 * AProjectileBullet
 * 무기(Gameplay Ability)에서 발사되어 실제 물리적 충돌과 데미지 전달을 담당하는 클래스입니다.
 */
UCLASS()
class FINALMINUTES_API AProjectileBullet : public AActor
{
    GENERATED_BODY()

public:
    AProjectileBullet();

protected:
    // 게임 시작 시 초기화
    virtual void BeginPlay() override;

    // 매 프레임 위치 추적 및 로직 처리
    virtual void Tick(float DeltaTime) override;

public:
    /** * [진입점] 무기 가동 시(GA_Fire 등) 호출됩니다. 
     * @param InSpecHandle : 적용할 데미지 정보가 담긴 GAS 스펙
     * @param InSpeed      : 발사 속도 (무기 데이터 에셋에서 전달)
     */
    virtual void InitializeProjectile(const FGameplayEffectSpecHandle& InSpecHandle, float InSpeed);

protected:
    /** * 충돌 발생 시 실행되는 콜백 함수 
     * 이 안에서 대상에게 GAS 이펙트를 적용하고 총알을 파괴합니다.
     */
    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                       FVector NormalImpulse, const FHitResult& Hit);

    /** 투사체 궤적 디버깅용 (필요 시 활성화) */
    void DrawDebugTrajectory();

protected:
    /* --- 컴포넌트 구성 --- */

    /** 최상단 루트 컴포넌트: 실제 충돌을 판정함 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> CollisionComp;

    /** 투사체의 물리적 이동(속도, 중력 등)을 관리함 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    /** 총알의 외형(Mesh) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BulletMesh;

    /* --- 데이터 및 상태 --- */

    /** 무기(GA)로부터 주입받은 데미지 정보 (충돌 시 대상의 ASC에 적용) */
    FGameplayEffectSpecHandle DamageEffectSpecHandle;

    /** 이전 프레임 위치 (매우 빠른 탄환의 터널링 현상 방지 및 궤적 계산용) */
    FVector LastLocation;

    /** 디버그 라인 표시 시간 */
    UPROPERTY(EditDefaultsOnly, Category = "Debug")
    float DebugLineDuration = 10.0f;
};