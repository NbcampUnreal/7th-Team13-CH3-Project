// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controller/AIBaseController.h"
#include "BTController.generated.h"

UCLASS()
class FINALMINUTES_API ABTController : public AAIBaseController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABTController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
