#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

struct FStreamableHandle;
enum class EWeaponActionType : uint8;
class UWeaponAttributeSet;
class UWeaponDataAsset;
class USkeletalMeshComponent;
class UAbilitySystemComponent;

UCLASS()
class FINALMINUTES_API AWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    AWeaponBase();

    /** 무기 초기 데이터 설정 및 에셋 로딩 시작 */
    void InitializeWeapon(FGameplayTag InWeaponTag, AActor* InOwner);

    /** 현재 탄약 수치 및 GAS 어트리뷰트 동기화 */
    void InitializeAttributes();

    /** 특정 액션(사격/재장전)에 따른 사운드 및 이펙트 실행 */
    void ExecuteWeaponEffects(EWeaponActionType ActionType);

    /** 캐릭터 손 소켓에 부착 및 관련 태그 부여 */
    void AttachToCharacter();

    /** 캐릭터 손에서 분리 및 관련 태그 제거 */
    void DetachFromCharacter();

    // -- Getters & Setters --
    FORCEINLINE UWeaponDataAsset* GetCurrentDataAsset() const { return CurrentDataAsset; }
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
    void SetCurrentAmmoCount(int32 InAmmo) { CurrentAmmoCount = InAmmo; }
    FVector GetMuzzleLocation() const;

protected:
    /** 메시 비동기 로딩 완료 후 호출되는 콜백 */
    virtual void OnWeaponMeshLoaded(FGameplayTag InWeaponTag);

private:
    /** 내부 로직 분리: 메시 로딩 시작 */
    void StartAsyncMeshLoad(FGameplayTag InWeaponTag);

    /** 내부 로직 분리: 소유자 ASC 가져오기 */
    UAbilitySystemComponent* GetOwnerASC() const;

    /** 내부 로직 분리: 장착 태그 관리 */
    void UpdateOwnerGameplayTag(bool bAddTag);

    /** 내부 로직 분리: 사운드 재생 */
    void PlayActionSound(EWeaponActionType ActionType);

    /** 내부 로직 분리: VFX 재생 */
    void SpawnActionEffects();

public:
    /** 현재 무기 활성화 여부 (장착 상태) */
    bool bIsActiveWeapon = false;

    /** 무기가 개별적으로 보유한 현재 탄약 수 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    int32 CurrentAmmoCount = 0;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;

    UPROPERTY(Transient)
    TObjectPtr<UWeaponDataAsset> CurrentDataAsset;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> WeaponOwner;

    /** 비동기 로딩 핸들 */
    TSharedPtr<FStreamableHandle> WeaponMeshLoadHandle;
};
