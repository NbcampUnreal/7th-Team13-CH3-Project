#include "UI/PlayerStatusWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"

void UPlayerStatusWidget::InitWithASC(UAbilitySystemComponent* InASC)
{
	ASC = InASC;
	if (!ASC) return;

	const UCharacterAttributeSet* AS = ASC->GetSet<UCharacterAttributeSet>();
	if (!AS) return;

	// Max는 고정이므로 1회만 저장
	MaxHealth  = FMath::Max(AS->GetMaxHealth(), 1.f);
	MaxStamina = FMath::Max(AS->GetMaxStamina(), 1.f);

	// UI 초기값 즉시 반영
	UpdateHealth(AS->GetHealth());
	UpdateStamina(AS->GetStamina());

	BindCallbacks(); // 델리게이트 바인딩
}

// 델리게이트에 함수 등록(AddUObject)
void UPlayerStatusWidget::BindCallbacks()
{
	if (!ASC) return;
	// Health 값이 바뀌면 OnHealthChanged 함수 호출
	HealthChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UPlayerStatusWidget::OnHealthChanged);

	StaminaChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetStaminaAttribute())
		.AddUObject(this, &UPlayerStatusWidget::OnStaminaChanged);
}

// 위젯이 사라질 때 델리게이트 연결 해제
void UPlayerStatusWidget::NativeDestruct()
{
	UnbindCallbacks();
	Super::NativeDestruct();
}

// 델리게이트 해제(Remove)
void UPlayerStatusWidget::UnbindCallbacks()
{
	if (!ASC) return;

	ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetHealthAttribute())
		.Remove(HealthChangedHandle);

	ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetStaminaAttribute())
		.Remove(StaminaChangedHandle);
}

//Health가 바뀌는 순간 자동 호출
void UPlayerStatusWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealth(Data.NewValue); // 변경된 체력
}

//Stamina가 바뀌는 순간 자동 호출
void UPlayerStatusWidget::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	UpdateStamina(Data.NewValue); // 변경된 스테미나
}

// 퍼센트 계산
void UPlayerStatusWidget::UpdateHealth(float Current)
{
	const float Percent = (MaxHealth > 0.f) ? FMath::Clamp(Current / MaxHealth, 0.f, 1.f) : 0.f;
	if (PB_Health) PB_Health->SetPercent(Percent);
}

void UPlayerStatusWidget::UpdateStamina(float Current)
{
	const float Percent = (MaxStamina > 0.f) ? FMath::Clamp(Current / MaxStamina, 0.f, 1.f) : 0.f;
	if (PB_Stamina) PB_Stamina->SetPercent(Percent);
}
