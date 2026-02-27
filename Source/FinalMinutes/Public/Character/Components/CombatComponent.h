// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"  // 추가
#include "Items/Weapons/FWeaponData.h" // 슬롯 Enum 포함
#include "CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveWeaponTagChanged, FGameplayTag, WeaponTag);

// 전방 선언: 컴파일 속도 최적화 및 의존성 최소화
struct FGameplayTag;
class APlayerCharacter;
class AWeaponBase;
class UAbilitySystemComponent;

/**
 * UCombatComponent
 * 캐릭터의 전투 로직 및 무기 인벤토리를 관리하는 핵심 컴포넌트입니다.
 * 주무기/보조무기 슬롯 시스템을 지원하며 현재 활성화된 무기를 제어합니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FINALMINUTES_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatComponent();

protected:
    // 초기화 및 매 프레임 업데이트 로직
    virtual void BeginPlay() override;

public:
    /** --- [정보 조회: Getter] --- */

    /** 현재 실제로 손에 들고 사격 가능한 무기를 반환합니다. */
    FORCEINLINE AWeaponBase* GetActiveWeapon() const { return ActiveWeapon; }
    
    /** 특정 인벤토리 슬롯(Primary/Secondary)에 저장된 무기 포인터를 조회합니다. */
    AWeaponBase* GetWeaponBySlot(const EWeaponSlot Slot) const;

    /** --- [전투 로직: 핵심 함수] --- */

    /** 외부(상호작용 시스템 등)에서 무기 태그를 통해 새로운 무기를 장착할 때 호출합니다. */
    UFUNCTION(BlueprintCallable, Category = "Combat | Actions")
    void EquipWeapon(FGameplayTag Tag);
    
    /** 1번, 2번 숫자 키 입력 등에 대응하여 특정 슬롯의 무기를 꺼냅니다. */
    UFUNCTION(BlueprintCallable, Category = "Combat | Actions")
    void SwapToSlot(EWeaponSlot TargetSlot);

    /** 단일 키 입력을 통한 주무기/보조무기 간 빠른 전환을 수행합니다. */
    UFUNCTION(BlueprintCallable, Category = "Combat | Actions")
    void ToggleWeaponSwap();
    
    // UI/HUD가 구독할 이벤트
    UPROPERTY(BlueprintAssignable, Category="Combat|UI")
    FOnActiveWeaponTagChanged OnActiveWeaponTagChanged;

private:
    /** --- [내부 유틸리티] --- */

    /** 실제 사격 판정이나 조준점을 찾기 위한 물리 레이캐스트를 수행합니다. */
    void PerformTrace();
    
    /** 소유자 캐릭터의 Ability System Component를 안전하게 가져오는 헬퍼 함수입니다. */
    UAbilitySystemComponent* GetOwnerASC() const;

private:
    /** 소유 캐릭터(PlayerCharacter) 참조 캐싱 */
    UPROPERTY()
    TObjectPtr<APlayerCharacter> OwnerCharacter;

    /** --- [무기 인벤토리 시스템] --- */

    /** [인벤토리] 주무기 슬롯: 라이플, 샷건 등 무거운 화기를 보관합니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Inventory", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AWeaponBase> PrimaryWeapon;

    /** [인벤토리] 보조무기 슬롯: 권총 등 가벼운 화기를 보관합니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Inventory", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AWeaponBase> SecondaryWeapon;

    /** [상태] 현재 활성화된 무기: Primary 혹은 Secondary 중 하나를 참조합니다. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Inventory", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AWeaponBase> ActiveWeapon;

    /** [상태] 현재 캐릭터가 선택 중인 슬롯 상태를 저장합니다. (None, Primary, Secondary) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Inventory", meta = (AllowPrivateAccess = "true"))
    EWeaponSlot CurrentSlot = EWeaponSlot::None;
};