// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "SensorAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class FINALMINUTES_API USensorAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	USensorAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// 소리 (발소리, 총기 소리에 활용)
	UPROPERTY(BlueprintReadOnly, Category = "Sensor|Sound")
	FGameplayAttributeData Loudness;
	ATTRIBUTE_ACCESSORS(USensorAttributeSet, Loudness);

	// 소음 반경
	UPROPERTY(BlueprintReadOnly, Category = "Sensor|Sound")
	FGameplayAttributeData SoundRadius;
	ATTRIBUTE_ACCESSORS(USensorAttributeSet, SoundRadius);

	// 냄새
	UPROPERTY(BlueprintReadOnly, Category = "Sensor|Scent")
	FGameplayAttributeData ScentIntensity;
	ATTRIBUTE_ACCESSORS(USensorAttributeSet, ScentIntensity);

	// 냄새 반경
	UPROPERTY(BlueprintReadOnly, Category = "Sensor|Scent")
	FGameplayAttributeData ScentRadius;
	ATTRIBUTE_ACCESSORS(USensorAttributeSet, ScentRadius);
};
