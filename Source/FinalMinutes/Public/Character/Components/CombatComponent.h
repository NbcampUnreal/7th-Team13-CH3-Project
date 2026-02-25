// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Weapons/FWeaponData.h"
#include "CombatComponent.generated.h"

struct FGameplayTag;
class APlayerCharacter;
class AWeaponBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FINALMINUTES_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatComponent();

protected:
    virtual void BeginPlay() override;
    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** [변경] 이제 'CurrentWeapon' 대신 'ActiveWeapon'을 반환합니다. */
    FORCEINLINE class AWeaponBase* GetActiveWeapon() const { return ActiveWeapon; }
    
    /** [추가] 특정 슬롯에 보관된 무기를 가져옵니다. */
    class AWeaponBase* GetWeaponBySlot(EWeaponSlot Slot) const;

    // 무기 장착 (3단계에서 로직 리팩터링 예정)
    UFUNCTION(BlueprintCallable)
    void EquipWeapon(FGameplayTag Tag);
    
    /** 특정 슬롯으로 무기를 교체합니다. (1번, 2번 키 입력 대응) */
    UFUNCTION(BlueprintCallable)
    void SwapToSlot(EWeaponSlot TargetSlot);

    /** 현재 슬롯이 아닌 다른 슬롯으로 무기를 전환합니다. (단일 키 토글용) */
    UFUNCTION(BlueprintCallable)
    void ToggleWeaponSwap();

private:
    /** 레이캐스트 수행 및 충돌 처리 */
    void PerformTrace();
    
    /** 소유자 ASC를 안전하게 가져오는 헬퍼 함수 */
    class UAbilitySystemComponent* GetOwnerASC() const;

private:
    /** 소유 캐릭터 참조 캐싱 */
    UPROPERTY()
    TObjectPtr<APlayerCharacter> OwnerCharacter;

    /* --- [2단계 추가 내용] 무기 인벤토리 저장소 --- */

    /** [주무기 슬롯] 라이플, 샷건 등 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Inventory", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AWeaponBase> PrimaryWeapon;

    /** [보조무기 슬롯] 권총 등 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Inventory", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AWeaponBase> SecondaryWeapon;

    /** [활성화된 무기] 현재 손에 들고 사격 가능한 무기 (Primary 혹은 Secondary를 가리킴) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Inventory", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AWeaponBase> ActiveWeapon;

    /** 현재 플레이어가 선택 중인 슬롯 상태 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Inventory", meta = (AllowPrivateAccess = "true"))
    EWeaponSlot CurrentSlot = EWeaponSlot::None;
};