#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CharacterAttributeSet.generated.h"

// 게터, 세터, 초기화 함수 자동 생성 매크로
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class FINALMINUTES_API UCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UCharacterAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	// 체력
	UPROPERTY(BlueprintReadOnly, Category = "Character|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, Health);

	// 최대 체력
	// UI HealthBar 표시용
	UPROPERTY(BlueprintReadOnly, Category = "Character|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MaxHealth);

	// 공격력
	UPROPERTY(BlueprintReadOnly, Category = "Character|Combat")
	FGameplayAttributeData AttackDamage;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, AttackDamage);

	// 방어력
	UPROPERTY(BlueprintReadOnly, Category = "Character|Combat")
	FGameplayAttributeData Defence;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, Defence);

	// 스테미너
	UPROPERTY(BlueprintReadOnly, Category = "Character|Stamina")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, Stamina);

	// 최대 스태미너
	UPROPERTY(BlueprintReadOnly, Category = "Character|Stamina")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MaxStamina);

	// 이동속도
	UPROPERTY(BlueprintReadOnly, Category = "Character|Stats")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MoveSpeed);
};
