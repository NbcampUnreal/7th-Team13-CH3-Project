#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

enum class EWeaponActionType : uint8;
class UWeaponAttributeSet;
class UWeaponDataAsset;
class USkeletalMeshComponent;
class UGameplayEffect;

UCLASS()
class FINALMINUTES_API AWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    AWeaponBase();

    /** 무기 초기화 및 비동기 메시 로딩 시작 */
    void InitializeWeapon(FGameplayTag InWeaponTag, AActor* InOwner);

    /** 데이터 테이블의 수치를 기반으로 GAS 어트리뷰트 초기화 */
    void InitializeAttributes();

    /** 사운드 및 VFX 실행 */
    void ExecuteWeaponEffects(EWeaponActionType ActionType);

    /** 캐릭터 손 소켓에 부착 및 태그 부여 */
    void AttachToCharacter();

    /** 캐릭터 손에서 분리 및 태그 제거 */
    void DetachFromCharacter();

    // Getter
    FORCEINLINE UWeaponDataAsset* GetCurrentDataAsset() const { return CurrentDataAsset; }
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
    FVector GetMuzzleLocation() const;

protected:
    /** 메시 로딩 완료 콜백 - 활성화 플래그에 따라 부착 여부 결정 */
    virtual void OnWeaponMeshLoaded(FGameplayTag InWeaponTag);

public:
    /** 현재 이 무기가 손에 들려야 하는 상태인지 여부 (CombatComponent에서 설정) */
    bool bIsActiveWeapon = false;

    /** 무기가 개별적으로 기억하는 현재 탄약 수치 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 CurrentAmmoCount = 0;

    // 현재 탄수 수동 설정 (CombatComponent에서 사용)
    void SetCurrentAmmoCount(int32 InAmmo) { CurrentAmmoCount = InAmmo; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;

    UPROPERTY(Transient)
    TObjectPtr<UWeaponDataAsset> CurrentDataAsset;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> WeaponOwner;

    /** 무기 스탯 초기화용 GE 클래스 (SetByCaller 활용) */
    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TSubclassOf<UGameplayEffect> InitStatEffectClass;

    FActiveGameplayEffectHandle WeaponStatEffectHandle;
};
