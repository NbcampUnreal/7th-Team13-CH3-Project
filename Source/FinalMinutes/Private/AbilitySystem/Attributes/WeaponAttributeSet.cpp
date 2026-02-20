// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "GameplayEffectExtension.h" // PostGameplayEffectExecute

UWeaponAttributeSet::UWeaponAttributeSet()
{
}

void UWeaponAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
    // 1. 탄약 관련 제한
    if (Attribute == GetCurrentAmmoAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAmmo());
    }
    else if (Attribute == GetMaxAmmoAttribute())
    {
        // 탄창은 최소 1발은 들어가야 시스템이 작동함
        NewValue = FMath::Max(NewValue, 1.0f);
    }
    
    // 2. 사격 간격 및 속도 관련 (0 이하 방지)
    else if (Attribute == GetFireRateAttribute() || Attribute == GetReloadSpeedAttribute() || Attribute == GetAdsSpeedAttribute())
    {
        // 0.01 이하로 떨어지면 시스템 로직상 위험하므로 최소값 유지
        NewValue = FMath::Max(NewValue, 0.01f);
    }
    
    // 3. 공격력, 총알 속도, 유효 사거리, 소음 크기, 탄퍼짐 값 제한
    else if (Attribute == GetDamageAttribute() || Attribute == GetBulletSpeedAttribute() || 
             Attribute == GetEffectiveRangeAttribute() || Attribute == GetSoundMultiplierAttribute() || 
             Attribute == GetCurrentSpreadAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.0f);
    }
}

void UWeaponAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
    const FGameplayAttribute ModifiedAttribute = Data.EvaluatedData.Attribute;

    // 현재 탄약 변경 시 클램핑: 0 <= CurrentAmmo <= MaxAmmo
    if (ModifiedAttribute == GetCurrentAmmoAttribute())
    {
        SetCurrentAmmo(FMath::Clamp(GetCurrentAmmo(), 0.0f, GetMaxAmmo()));
    }

    // 최대 탄창 크기가 변했을 때 현재 탄약량 재조정
    if (ModifiedAttribute == GetMaxAmmoAttribute())
    {
        // MaxAmmo 하한선 보호
        SetMaxAmmo(FMath::Max(GetMaxAmmo(), 1.0f));
        
        // 탄창이 줄어들었다면 현재 탄약도 그에 맞게 줄임
        if (GetCurrentAmmo() > GetMaxAmmo())
        {
            SetCurrentAmmo(GetMaxAmmo());
        }
    }

    // 공격력 및 속도 계열 최종 검증
    if (ModifiedAttribute == GetDamageAttribute())
    {
        SetDamage(FMath::Max(GetDamage(), 0.0f));
    }
    
    if (ModifiedAttribute == GetFireRateAttribute())
    {
        SetFireRate(FMath::Max(GetFireRate(), 0.01f));
    }
}
