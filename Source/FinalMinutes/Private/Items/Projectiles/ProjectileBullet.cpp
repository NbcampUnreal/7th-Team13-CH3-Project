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
    
	// 1. 물리 충돌체 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(5.0f);
    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
    CollisionComp->OnComponentHit.AddDynamic(this, &AProjectileBullet::OnHit);
    CollisionComp->SetNotifyRigidBodyCollision(true);
    CollisionComp->BodyInstance.bUseCCD = true;
    RootComponent = CollisionComp;

    // 2. 비주얼 설정
    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
    BulletMesh->SetupAttachment(RootComponent);
    BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 3. 발사체 이동 컴포넌트 (초기 속도는 0, Initialize에서 설정)
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    InitialLifeSpan = 10.0f; // 3초 뒤 자동 소멸
}

void AProjectileBullet::InitializeProjectile(const FGameplayEffectSpecHandle& InSpecHandle, float InSpeed)
{
	DamageEffectSpecHandle = InSpecHandle;
    
    if (ProjectileMovement)
    {
        UE_LOG(LogTemp, Warning, TEXT("Projectile Speed: %f"), InSpeed);
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
	
}

// Called every frame
void AProjectileBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

