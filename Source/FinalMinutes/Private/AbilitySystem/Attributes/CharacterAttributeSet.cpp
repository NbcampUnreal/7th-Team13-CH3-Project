#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "GameplayEffectExtension.h"

UCharacterAttributeSet::UCharacterAttributeSet()
{
    // 초기화
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
	   
    InitStamina(100.0f);
    InitMaxStamina(100.0f);
    
    InitAttackDamage(10.0f);
    InitDefence(0.0f);
    InitMoveSpeed(600.0f);
}

// Attribute값이 변하기 전에 실행
void UCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);
	
    if (Attribute == GetMaxHealthAttribute())
    {
        // 최대체력 보정 (최대 체력은 1보다 작을 수 없음)
        NewValue = FMath::Max(NewValue, 1.0f);
    }
    else if (Attribute == GetMoveSpeedAttribute())
    {
        // 이동속도 보정 (이동속도는 0보다 작을 수 없음)
        NewValue = FMath::Max(NewValue, 0.0f);
    }
}

// Attribute값이 바뀌고 나서 실행
void UCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
	
    // 현재 변경된 Attribute가 무엇인지 확인한다. Health면 Health에 맞게 보정처리
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // GetHealth값이 최소 0.0f, 최대 GetMaxHealth를 넘지 않게 보정해준다.
        // ex) 최대체력 100, 현재 체력90일때 체력 20을 회복시켜주는 힐팩을 먹음 -> 현재체력 110 -> 최대체력 넘음 (x) 잘못됨
        // 그래서 보정을 해줌 0.0f에서 최대체력 100사이
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
    }
    else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
    }
}

