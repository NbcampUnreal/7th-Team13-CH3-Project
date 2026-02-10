#include "AbilitySystem/Attributes/SensorAttributeSet.h"

USensorAttributeSet::USensorAttributeSet()
{
	// 기본값 초기화
	InitLoudness(0.0f);
	InitSoundRadius(500.0f); // 기본 발소리 범위 5m (언리얼 단위 500)

	InitScentIntensity(10.0f);
	InitScentRadius(300.0f); // 3m 범위로 냄새 확산
}

void USensorAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetLoudnessAttribute() || Attribute == GetScentIntensityAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetSoundRadiusAttribute() || Attribute == GetScentRadiusAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}