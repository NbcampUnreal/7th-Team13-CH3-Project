// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/BTController.h"

// Sets default values
ABTController::ABTController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABTController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABTController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

