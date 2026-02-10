// Fill out your copyright notice in the Description page of Project Settings.


#include "FinalMinutes/Public/Items/Weapons/WeaponBase.h"
#include "FinalMinutes/Public/Items/Weapons/WeaponDataAsset.h"
#include "FinalMinutes/Public/AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h" // 비동기 로딩


AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// 메쉬 컴포넌트 생성 및 루트 설정
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);
    
    // 초기에는 보이지 않게 설정 (로드 완료 후 활성화)
    WeaponMesh->SetHiddenInGame(true);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::InitializeWeapon(UWeaponDataAsset* InDataAsset, UWeaponAttributeSet* InAttributes)
{
	if (!InDataAsset || !InAttributes) return;

    WeaponData = InDataAsset;
    WeaponAttributes = InAttributes;
    WeaponTag = WeaponData->WeaponTag;

    // PrimaryDataAsset에서 스탯 로드
    WeaponAttributes->InitDamage(WeaponData->DefaultDamage);
    WeaponAttributes->InitMaxAmmo(WeaponData->DefaultMaxAmmo);
    WeaponAttributes->InitCurrentAmmo(WeaponData->DefaultMaxAmmo);
    WeaponAttributes->InitFireRate(WeaponData->DefaultFireRate);
    WeaponAttributes->InitBulletSpeed(WeaponData->DefaultBulletSpeed);
    WeaponAttributes->InitReloadSpeed(WeaponData->DefaultReloadSpeed);
    WeaponAttributes->InitSoundMultiplier(1.0f); // 소음 배율 초기화

    // 성능 저하 없는 에셋 로딩
    if (WeaponData->WeaponMesh.IsPending())
    {
        // 에셋 매니저를 통한 비동기 요청
        FStreamableManager& Streamable = UAssetManager::Get().GetStreamableManager();
        Streamable.RequestAsyncLoad(WeaponData->WeaponMesh.ToSoftObjectPath(), 
            FStreamableDelegate::CreateUObject(this, &AWeaponBase::OnWeaponMeshLoaded));
    }
    else
    {
        // 이미 메모리에 로드되어 있는 경우 즉시 실행
        OnWeaponMeshLoaded();
    }
}

void AWeaponBase::OnWeaponMeshLoaded() const
{
	if (WeaponData && WeaponMesh)
    {
        // 로드된 소프트 포인터로부터 메쉬를 가져와 적용
        if (USkeletalMesh* LoadedMesh = WeaponData->WeaponMesh.Get())
        {
            WeaponMesh->SetSkeletalMesh(LoadedMesh);
            WeaponMesh->SetHiddenInGame(false); // 비주얼 활성화
            
            UE_LOG(LogTemp, Log, TEXT("[%s] Mesh Async Loaded Successfully."), *WeaponData->WeaponName.ToString());
        }
    }
}

void AWeaponBase::ActivateWeapon()
{
    SetActorHiddenInGame(false);
    SetActorTickEnabled(true);
}

void AWeaponBase::DeactivateWeapon()
{
    SetActorHiddenInGame(true);
    SetActorTickEnabled(false);
}




