// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "WeaponData.generated.h"

class UNiagaraSystem;

UENUM(BlueprintType)
enum class EWeaponActionType : uint8
{
    Fire,
    Reload,
    Equip
};

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
    Primary UMETA(DisplayName = "Primary (Rifle/Shotgun/Sniper/Grenade Launcher)"),
    Secondary UMETA(DisplayName = "Secondary (Pistol)"),
    None UMETA(DisplayName = "None")
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FINALMINUTES_API FWeaponData : public FTableRowBase
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

    /** 무기가 장착될 기본 슬롯 설정 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    EWeaponSlot DefaultSlot = EWeaponSlot::None;
#pragma endregion

#pragma region 비주얼 및 오디오 에셋
    /** * 무기 메시 (비동기 로딩을 위해 Soft Pointer 사용)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual")
    TSoftObjectPtr<USkeletalMesh> WeaponMesh;

    /** 무기별 미세 조정용 트랜스폼 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Visual")
    FTransform WeaponMeshOffset = FTransform(FRotator(-8.06f, -77.8f, 22.03f), FVector(-10.0f, -0.8f, 8.99f));

    /** 시각적인 이펙트 */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Visual")
    TSoftObjectPtr<UNiagaraSystem> MuzzleFlash; // 총구 화염 (Niagara)

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Visual")
    TSoftObjectPtr<UNiagaraSystem> ImpactEffect; // 피격 이펙트

    /** 발사 및 재장전 사운드 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Audio")
    TSoftObjectPtr<USoundBase> FireSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Audio")
    TSoftObjectPtr<USoundBase> ReloadSound;
#pragma endregion

#pragma region 소켓 및 투사체
    /** 캐릭터 손에 붙을 소켓 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
    FName HandSocketName = FName("hand_r");

    /** 총알이 나갈 총구 소켓 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
    FName MuzzleSocketName = FName("Muzzle");

    /** 탄피가 배출될 소켓 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Socket")
    FName EjectSocketName = NAME_None;

    /** 생성할 투사체 클래스 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
    TSubclassOf<AActor> ProjectileClass;
#pragma endregion

#pragma region 초기 스탯 (AttributeSet 초기화용)
    /** 기본 탄창 크기 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 DefaultMaxAmmo = 30;

    /** 기본 공격력 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float DefaultDamage = 20.0f;

    /** 연사 가능 여부 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    bool bIsFullAuto = true;

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

    /** 소음기 장착 시 소음 감소율 (예) 0.3f = 30%로 감소 */
    UPROPERTY(EditAnywhere, Category = "Combat|Mod")
    float SuppressorSoundMultiplier = 1.0f;

    /** 무기별 기본 반동 세기 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float DefaultRecoilValue = 1.5f;

    /**
     * 유효 사거리(cm)
     * - 투사체가 이 거리보다 멀리 비행하면 파괴됩니다.
     * - 데미지 감쇄도 이 거리를 기준으로 선형 적용됩니다(가까울수록 강, 멀수록 약).
     *   (0 이하이면 제한/감쇄를 적용하지 않음)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float EffectiveRange = 5000.0f;
#pragma endregion

#pragma region 샷건 / 탄퍼짐 설정
    /**
     * - false: 기존처럼 1발(단일 투사체)만 발사
     * - true : PelletCount 만큼 투사체를 스폰하고 퍼짐 적용
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun")
    bool bUsePelletSpread = false;

    /** 한 번 발사할 때 생성할 투사체 개수 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun", meta = (ClampMin = "1", UIMin = "1"))
    int32 PelletCount = 1;

    /**
     * 투사체 퍼짐 반각(도 단위). (예: 6도면 중심선 기준 -6~+6도 안에서 랜덤)
     * bUsePelletSpread가 true일 때 적용됩니다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun",
        meta = (ClampMin = "0.0", UIMin = "0.0"))
    float PelletSpreadHalfAngleDeg = 6.0f;

    /**
     * 총알당 데미지 배수
     * - DefaultDamage를 "한 번 방아쇠 총합 데미지"로 사용.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun",
        meta = (ClampMin = "0.0", UIMin = "0.0"))
    float PelletDamageMultiplier = 1.0f / PelletCount;

#pragma endregion

    // 사격 어빌리티 설정
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Data")
    TSubclassOf<class UGameplayAbility> FireAbilityClass;
    
    // 총알 ID
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Ammo")
    FName AmmoItemID = NAME_None;
};
