// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponDataAsset.generated.h"

/**
 * 무기 제작을 위한 정적 설계도 클래스
 * UPrimaryDataAsset을 상속받아 AssetManager의 비동기 로딩 기능을 활용합니다.
 */
UCLASS(BlueprintType)
class FINALMINUTES_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	public:
#pragma region 기본 정보 및 식별
    /** 무기의 UI 표시용 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Info")
    FText WeaponName;

    /** 무기 식별 및 시스템 연동용 태그 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Info")
    FGameplayTag WeaponTag;
#pragma endregion

#pragma region 비주얼 및 오디오 에셋
    /** * 무기 메시 (비동기 로딩을 위해 Soft Pointer 사용)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual")
    TSoftObjectPtr<USkeletalMesh> WeaponMesh;

    /** 발사 및 재장전 사운드 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Audio")
    TSoftObjectPtr<USoundBase> FireSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Audio")
    TSoftObjectPtr<USoundBase> ReloadSound;

    /** 애니메이션 몽타주 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> FireMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> ReloadMontage;
#pragma endregion

#pragma region 소켓 및 투사체
    /** 캐릭터 손에 붙을 소켓 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
    FName HandSocketName = FName("WeaponSocket_R");

    /** 총알이 나갈 총구 소켓 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
    FName MuzzleSocketName = FName("Muzzle");

    /** 탄피가 배출될 소켓 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
    FName EjectSocketName = FName("Eject");

    /** 생성할 투사체 클래스 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
    TSubclassOf<AActor> ProjectileClass;
#pragma endregion

#pragma region 초기 스탯 (AttributeSet 초기화용)
	/** 기본 탄창 크기 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float DefaultMaxAmmo = 30.0f;

	/** 기본 공격력 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float DefaultDamage = 20.0f;

	/** 기본 연사 속도 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float DefaultFireRate = 0.2f;

	/** 기본 재장전 속도 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float DefaultReloadSpeed = 1.0f;

	/** 기본 총알 속도 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float DefaultBulletSpeed = 5000.0f;

    /** 사격 시 발생하는 기본 소음 범위 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float DefaultSoundSize = 1000.0f;

    /** 무기별 기본 반동 세기 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float DefaultRecoilValue = 1.5f;

    /** 무기별 최대 탄퍼짐 각도 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float MaxSpreadAngle = 5.0f;
#pragma endregion

    /** Asset Manager에서 이 에셋을 고유하게 식별하기 위한 ID 반환 */
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(FName("WeaponData"), GetFName());
    }
};
