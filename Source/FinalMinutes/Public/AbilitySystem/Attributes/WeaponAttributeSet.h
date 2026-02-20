// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "WeaponAttributeSet.generated.h"

// GAS 표준 매크로: 수치 접근 및 초기화
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 무기 전용 속성 세트 클래스
 */
UCLASS()
class FINALMINUTES_API UWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UWeaponAttributeSet();
	
#pragma region AttributeSet의 라이프사이클 관리 함수
	// 속성값이 변경되기 직전에 호출되어 값을 제한
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

    // GameplayEffect가 실행된 후 호출되어 최종 값을 확정
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
#pragma endregion
	
#pragma region 탄약 속성
	/** 탄약 관련 속성 */
	// 현재 탄약 수
	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	FGameplayAttributeData CurrentAmmo;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentAmmo)

	// 탄창 크기
	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	FGameplayAttributeData MaxAmmo;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxAmmo)
#pragma endregion

#pragma region 전투 속성
	/** 전투 성능 관련 속성 */
	// 현재 무기 공격력
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, Damage)

	// 현재 연사 속도
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData FireRate;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, FireRate)

	// 현재 재장전 속도
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData ReloadSpeed;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, ReloadSpeed)

	// 현재 총알 속도
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData BulletSpeed;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, BulletSpeed)
#pragma endregion 
	
#pragma region 특수 및 유틸리티 속성
	/** 특수 및 유틸리티 속성 */
	// 현재 소음 크기
	UPROPERTY(BlueprintReadOnly, Category = "Utility")
	FGameplayAttributeData SoundMultiplier;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, SoundMultiplier)

	// 현재 유효 사거리
	UPROPERTY(BlueprintReadOnly, Category = "Utility")
	FGameplayAttributeData EffectiveRange;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, EffectiveRange)

	// 현재 조준 속도
	UPROPERTY(BlueprintReadOnly, Category = "Utility")
	FGameplayAttributeData AdsSpeed;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, AdsSpeed)

	// 현재 탄퍼짐 계수
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData CurrentSpread;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentSpread)
#pragma endregion
};
