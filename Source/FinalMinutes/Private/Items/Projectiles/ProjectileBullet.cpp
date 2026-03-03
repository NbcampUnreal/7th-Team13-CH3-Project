// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Projectiles/ProjectileBullet.h"

// 엔진 및 컴포넌트 헤더
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Physics/CharacterPhysicalMaterial.h"

// GAS 및 인터페이스 헤더
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Interfaces/Damageable.h"

// 디버그 헬퍼
#include "DrawDebugHelpers.h"

AProjectileBullet::AProjectileBullet()
{
    // TPS 게임의 총알은 매 프레임 궤적 추적이 필요하므로 Tick을 활성화합니다.
    PrimaryActorTick.bCanEverTick = true;

    // 1. 물리 충돌체(Sphere) 설정: 모든 물리 판정의 기준이 됩니다.
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(5.0f);

    // 프로젝트 설정의 'Projectile' 프로필 사용 (기능적 충돌 설정)
    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));

    // 빠른 탄속에서 벽을 뚫는 '터널링' 현상을 방지하기 위해 CCD(연속 충돌 감지)를 활성화합니다.
    CollisionComp->BodyInstance.bUseCCD = true;

    // 물리적 충돌 이벤트를 발생시키고 OnHit 함수를 바인딩합니다.
    CollisionComp->SetNotifyRigidBodyCollision(true);
    CollisionComp->OnComponentHit.AddDynamic(this, &AProjectileBullet::OnHit);
    CollisionComp->bReturnMaterialOnMove = true;

    RootComponent = CollisionComp;

    // 2. 비주얼(Mesh) 설정: 실제 충돌에는 관여하지 않고 시각 정보만 제공합니다.
    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
    BulletMesh->SetupAttachment(RootComponent);
    BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌 간섭 방지
    BulletMesh->bReturnMaterialOnMove = true;

    // 3. 발사체 이동 컴포넌트: 총알의 물리적 거동을 처리합니다.
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bRotationFollowsVelocity = true; // 이동 방향에 맞춰 총알 회전
    ProjectileMovement->bShouldBounce = false;

    // 네트워크 복제 설정 (멀티플레이어 대응)
    /*bReplicates = true;
    SetReplicateMovement(true);*/
}

void AProjectileBullet::BeginPlay()
{
    Super::BeginPlay();

    // 시작 위치 기록 (거리 기반 사거리 제한/데미지 감쇄용)
    SpawnLocation = GetActorLocation();
    // 시작 위치를 기록하여 Tick에서 궤적을 그릴 준비를 합니다.
    LastLocation = GetActorLocation();
}

/**
 * [Flow 1] 무기에서 발사 시 호출되어 총알에 '생명'을 불어넣습니다.
 */
void AProjectileBullet::InitializeProjectile(
    const FGameplayEffectSpecHandle& InSpecHandle,
    float InSpeed)
{
    // 무기로부터 넘어온 데미지 데이터(GAS Spec)를 저장합니다.
    DamageEffectSpecHandle = InSpecHandle;
    
    if (!ProjectileMovement) return;

    // 무기 데이터 에셋에서 정의된 속도로 물리 엔진 수치를 설정합니다.
    ProjectileMovement->InitialSpeed = InSpeed;
    ProjectileMovement->MaxSpeed = InSpeed;
    ProjectileMovement->Velocity = GetActorForwardVector() * InSpeed;

    // 발사 즉시 자기 자신(플레이어)과 부딪히는 것을 방지합니다.
    AActor* MyInstigator = GetInstigator();
    if (!MyInstigator) return;

    CollisionComp->IgnoreActorWhenMoving(MyInstigator, true);
}

void AProjectileBullet::IgnoreOtherProjectile(AActor* OtherProjectile)
{
    if (!OtherProjectile || OtherProjectile == this) return;
    if (!CollisionComp) return;

    CollisionComp->IgnoreActorWhenMoving(OtherProjectile, true);
}

/**
 * [Flow 2] 충돌 시 실행되며, 타겟에게 고통을 전달합니다.
 */
void AProjectileBullet::OnHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    // 1. 기본 방어 로직
    // 1. 자기 자신과 발사자 제외 (필수 최소 필터)
    if (!OtherActor || OtherActor == GetInstigator() || OtherActor == this) return;
    
    // 2. IDamageable 인터페이스 호출 (IDamageable 구현자라면 누구든)
    IDamageable* DamageableTarget = Cast<IDamageable>(OtherActor);
    if (DamageableTarget)
    {
        // 타겟인 몬스터에게 맞았다고 알려줌.
        DamageableTarget->Execute_OnHitReaction(OtherActor, Hit);
    }

    // 3. GAS 데이터 적용 (ASC 인터페이스 구현자라면 누구든)
    // 타겟이 GAS를 사용하는 클래스(IAbilitySystemInterface 상속)인지 확인합니다.
    IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(OtherActor);
    if (!ASCOwner && OtherActor->GetOwner())
    {
        ASCOwner = Cast<IAbilitySystemInterface>(OtherActor->GetOwner());
    }
    if (!ASCOwner) return;
    

    UAbilitySystemComponent* TargetASC = ASCOwner->GetAbilitySystemComponent();
    if (!TargetASC || !DamageEffectSpecHandle.IsValid()) return;

    // [Step 3] 실제 전달된 데미지 값 확인
    // "Data.Damage" 부분은 무기 담당자가 설정한 실제 태그명으로 교체해야 합니다.
    FGameplayEffectSpec* Spec = DamageEffectSpecHandle.Data.Get();
    if (Spec)
    {
        // 1. 부위 판별용 태그 변수
        FGameplayTag HitRegionTag;
        
        // 2. 물리적 충돌 정보(Hit.BoneName)를 확인하여 태그 결정
        if (Hit.BoneName.ToString().Contains(TEXT("head")))
        {
            HitRegionTag = FGameplayTag::RequestGameplayTag(FName("Damage.HitRegion.Head"));
        }
        else
        {
            // 머리가 아니면 기본적으로 몸통(Body)으로 간주
            HitRegionTag = FGameplayTag::RequestGameplayTag(FName("Damage.HitRegion.Body"));
        }
        
        // 3. Spec에 이 태그를 추가해서 전달
        Spec->AddDynamicAssetTag(HitRegionTag);
    }


    // 5. 거리 기반 데미지 감쇄(선형)
    //    - 0m: 100%
    //    - EffectiveRangeCm: 0%
    if (EffectiveRangeCm > 0.0f && Spec)
    {
        const float Traveled = FVector::Distance(SpawnLocation, Hit.ImpactPoint);
        const float Alpha = FMath::Clamp(Traveled / EffectiveRangeCm, 0.0f, 1.0f);
        const float DamageMultiplier = 1.0f - Alpha;

        const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Effect.Damage"));

        const float OriginalDamage = Spec->GetSetByCallerMagnitude(DamageTag, false, 0.0f);
        if (OriginalDamage > 0.0f)
        {
            Spec->SetSetByCallerMagnitude(DamageTag, OriginalDamage * DamageMultiplier);
        }
    }

    // 저장해둔 데미지 효과를 타겟의 ASC에 직접 적용합니다.
    TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
    
    // 충돌 후 총알 액터를 즉시 월드에서 제거합니다.
    Destroy();
}

void AProjectileBullet::DrawDebugTrajectory()
{
    // 서버에서는 디버그 라인을 그릴 필요가 없으므로 클라이언트/에디터 환경에서만 실행합니다.
    if (GetNetMode() == NM_DedicatedServer) return;

    const FVector CurrentLocation = GetActorLocation();

    // 이전 위치와 현재 위치 사이에 선을 그어 탄도를 확인합니다.
    DrawDebugLine(
        GetWorld(),
        LastLocation,
        CurrentLocation,
        FColor::Red,
        false,
        DebugLineDuration,
        0,
        1.5f // 가시성을 위해 두께를 조금 높임
    );

    // 다음 프레임을 위해 현재 위치를 업데이트합니다.
    LastLocation = CurrentLocation;
}

/**
 * [Flow 3] 매 프레임 이동 궤적을 시각적으로 추적합니다. (디버그 및 피드백용)
 */
void AProjectileBullet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // 유효 사거리 제한: 사거리 밖으로 나가면 투사체 제거
    if (EffectiveRangeCm > 0.0f)
    {
        const float TraveledSq = FVector::DistSquared(SpawnLocation, GetActorLocation());
        const float RangeSq = EffectiveRangeCm * EffectiveRangeCm;

        if (TraveledSq >= RangeSq)
        {
            Destroy();
            return;
        }
    }

    // DrawDebugTrajectory();
}
