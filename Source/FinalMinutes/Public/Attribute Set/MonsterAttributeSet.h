#pragma once

// 이 코드 부분은 이후 CharacterAttributeSet에 아래의 2가지 데이터를 추가하고 나서 삭제하면 됩니다.

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.generated.h"

// GAS 속성 접근을 편하게 해주는 매크로 (표준)
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

class UAbilitySystemComponent;

UCLASS()
class FINALMINUTES_API UMonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UMonsterAttributeSet();
	
	// 부위별 방어력 (머리)
	UPROPERTY(BlueprintReadOnly, Category = "Monster|Def")
	FGameplayAttributeData HeadDefence;
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, HeadDefence)
	
	// 부위별 방어력 (몸통)
	UPROPERTY(BlueprintReadOnly, Category = "Monster|Def")
	FGameplayAttributeData BodyDefence;
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, BodyDefence)
};
