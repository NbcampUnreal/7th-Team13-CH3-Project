// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ProjectileBullet.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class FINALMINUTES_API AProjectileBullet : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectileBullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	// 발사체의 이동을 담당할 컴포넌트
	UPROPERTY(VisibleAnywhere, Category="Movement")
	TObjectPtr<UProjectileMovementComponent> movementComp;
	
	// 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, Category="Collision")
	TObjectPtr<USphereComponent> collisionComp;
	
	// 외관 컴포넌트
	UPROPERTY(VisibleAnywhere, Category="BodyMesh")
	TObjectPtr<UStaticMeshComponent> bodyMeshComp;
};
