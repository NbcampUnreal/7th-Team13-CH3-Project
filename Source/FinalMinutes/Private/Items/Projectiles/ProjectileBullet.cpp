// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Projectiles/ProjectileBullet.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
AProjectileBullet::AProjectileBullet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// 1. 충돌체 등록하기
	collisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	
	// 2. 충돌 프로파일 설정
	collisionComp->SetCollisionProfileName(TEXT("BlockAll"));
	
	// 3. 충돌체 크기 설정
	collisionComp->SetSphereRadius(13);
	
	// 4. 루트로 등록
	RootComponent = collisionComp;
	
	// 5. 외관 컴포넌트 등록하기
	bodyMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshComp"));
	
	// 6. 부모 컴포넌트 지정
	bodyMeshComp->SetupAttachment(collisionComp);
	
	// 7. 충돌 비활성화
	bodyMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 8. 외관 크기 설정
	bodyMeshComp->SetRelativeScale3D(FVector(0.05f));
	
	// 발사체 컴포넌트
	movementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));
	
	// movement 컴포넌트가 갱신시킬 컴포넌트 지정
	movementComp->SetUpdatedComponent(collisionComp);
	
	// 초기 속도
	movementComp->InitialSpeed = 5000.0f;
	
	// 최대 속도
	movementComp->MaxSpeed = 5000.0f;
	
	// 반동 여부
	movementComp->bShouldBounce = true;
	
	// 반동 값
	movementComp->Bounciness = 0.3f;
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

