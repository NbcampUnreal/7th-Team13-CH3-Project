// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Projectiles/ProjectileBullet.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
AProjectileBullet::AProjectileBullet()
{
    PrimaryActorTick.bCanEverTick = true;

    // 1. 물리 충돌체(Sphere) 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(5.0f);

    // 프로필 설정 후, 'Query(이벤트 감지)'가 가능하도록 명시적 설정
    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // 충돌 시 OnHit 이벤트가 발생하도록 설정 (Simulation Generates Hit Events)
    CollisionComp->SetNotifyRigidBodyCollision(true);

    // 빠른 탄속에서의 터널링 방지를 위한 CCD 활성화
    CollisionComp->BodyInstance.bUseCCD = true;

    CollisionComp->OnComponentHit.AddDynamic(this, &AProjectileBullet::OnHit);
    RootComponent = CollisionComp;

    // 2. 비주얼(Mesh) 설정
    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
    BulletMesh->SetupAttachment(RootComponent);

    // 메시는 충돌 연산을 하지 않고 시각 정보만 제공 (간섭 방지)
    BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BulletMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

    // 3. 발사체 이동 컴포넌트
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    // 수명 설정
    InitialLifeSpan = 10.0f;
}

void AProjectileBullet::InitializeProjectile(const FGameplayEffectSpecHandle& InSpecHandle, float InSpeed)
{
    DamageEffectSpecHandle = InSpecHandle;

    if (ProjectileMovement)
    {
        ProjectileMovement->InitialSpeed = InSpeed;
        ProjectileMovement->MaxSpeed = InSpeed;

        ProjectileMovement->Velocity = GetActorForwardVector() * InSpeed;
    }

    if (AActor* MyInstigator = GetInstigator())
    {
        // CollisionComponent가 MyInstigator(캐릭터)를 무시하도록 설정
        CollisionComp->IgnoreActorWhenMoving(MyInstigator, true);
    }
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && OtherActor != GetInstigator())
    {
        // 4. 타겟의 ASC를 찾아 미리 보관해둔 데미지 효과 적용
        if (IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(OtherActor))
        {
            UAbilitySystemComponent* TargetASC = ASCOwner->GetAbilitySystemComponent();
            if (TargetASC && DamageEffectSpecHandle.IsValid())
            {
                // 발사자(Instigator)의 ASC를 통해 타겟에게 효과 적용
                TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
            }
        }

        // 피격 이펙트 재생 로직이 들어갈 자리 (VFX/SFX)
    }

    // Destroy(); // 충돌 후 소멸
}

// Called when the game starts or when spawned
void AProjectileBullet::BeginPlay()
{
    Super::BeginPlay();

    // 시작 위치 초기화
    LastLocation = GetActorLocation();
}

void AProjectileBullet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector CurrentLocation = GetActorLocation();

    // 이전 위치에서 현재 위치까지 빨간색 선을 그림
    DrawDebugLine(
        GetWorld(),
        LastLocation,
        CurrentLocation,
        FColor::Red, // 색상
        false, // 지속성 여부
        DebugLineDuration, // 지속 시간
        0, // 우선 순위
        1.0f // 두께
    );

    // 다음 프레임을 위해 위치 갱신
    LastLocation = CurrentLocation;
}
