// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Projectiles/ProjectileBullet.h"

// 엔진 및 컴포넌트 헤더
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"

// GAS 및 인터페이스 헤더
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

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

    RootComponent = CollisionComp;

    // 2. 비주얼(Mesh) 설정: 실제 충돌에는 관여하지 않고 시각 정보만 제공합니다.
    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
    BulletMesh->SetupAttachment(RootComponent);
    BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌 간섭 방지

    // 3. 발사체 이동 컴포넌트: 총알의 물리적 거동을 처리합니다.
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bRotationFollowsVelocity = true; // 이동 방향에 맞춰 총알 회전
    ProjectileMovement->bShouldBounce = false;

    // 네트워크 복제 설정 (멀티플레이어 대응)
    bReplicates = true;
    SetReplicateMovement(true);
}

void AProjectileBullet::BeginPlay()
{
    Super::BeginPlay();

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
    // 타겟이 유효하고 내가 쏜 사람이 아닐 때만 로직 실행
    if (!OtherActor || OtherActor == GetInstigator()) return;

    // 타겟이 GAS를 사용하는 클래스(IAbilitySystemInterface 상속)인지 확인합니다.
    IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(OtherActor);
    if (!ASCOwner) return;

    UAbilitySystemComponent* TargetASC = ASCOwner->GetAbilitySystemComponent();
    if (!TargetASC || !DamageEffectSpecHandle.IsValid()) return;

    // 저장해둔 데미지 효과를 타겟의 ASC에 직접 적용합니다.
    TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());

    // TODO: 피격 위치(Hit.ImpactPoint)에 따른 이펙트(혈흔, 스파크 등) 생성

    // 충돌 후 총알 액터를 즉시 월드에서 제거합니다.
    Destroy();
}

/**
 * [Flow 3] 매 프레임 이동 궤적을 시각적으로 추적합니다. (디버그 및 피드백용)
 */
void AProjectileBullet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

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
