// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

struct FGameplayTag;
class APlayerCharacter;
class AWeaponBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FINALMINUTES_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	FORCEINLINE class AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }
	
	// 무기 장착
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(FGameplayTag Tag);
	
	// 무기 교체
	UFUNCTION(BlueprintCallable)
	void SwapWeapon();
private:
	/** 레이캐스트 수행 및 충돌 처리 */
    void PerformTrace();
	
	/** 소유자 ASC를 안전하게 가져오는 헬퍼 함수 */
    class UAbilitySystemComponent* GetOwnerASC() const;
	
	/** 소유 캐릭터 참조 캐싱 */
    UPROPERTY()
    TObjectPtr<APlayerCharacter> OwnerCharacter;
	
	UPROPERTY()
	TObjectPtr<AWeaponBase> CurrentWeapon;
};
