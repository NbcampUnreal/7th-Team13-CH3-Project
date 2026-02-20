#include "UI/PlayerStatusWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"

void UPlayerStatusWidget::InitWithASC(UAbilitySystemComponent* InASC)
{
	ASC = InASC;
	if (!ASC) return;

	// Character AttributeSet 가져오기
	const UCharacterAttributeSet* CAS = ASC->GetSet<UCharacterAttributeSet>();
	if (!CAS) return;
	
	// Weapon AttributeSet 가져오기
	const UWeaponAttributeSet* WAS = ASC->GetSet<UWeaponAttributeSet>();
	if (WAS)
	{
		UpdateAmmo(WAS->GetCurrentAmmo());
		UpdateMaxAmmo(WAS->GetMaxAmmo()); // 혹시 몰라서 최대 값도 불러옴
	}

	// Max는 고정이므로 1회만 저장
	MaxHealth  = FMath::Max(CAS->GetMaxHealth(), 1.f);
	MaxStamina = FMath::Max(CAS->GetMaxStamina(), 1.f);

	// UI 초기값 즉시 반영
	UpdateHealth(CAS->GetHealth());
	UpdateStamina(CAS->GetStamina());

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
	
	AmmoChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UWeaponAttributeSet::GetCurrentAmmoAttribute())
		.AddUObject(this, &UPlayerStatusWidget::OnAmmoChanged);
	
	MaxAmmoChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(UWeaponAttributeSet::GetMaxAmmoAttribute())
		.AddUObject(this, &UPlayerStatusWidget::OnMaxAmmoChanged);
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
	
	ASC->GetGameplayAttributeValueChangeDelegate(UWeaponAttributeSet::GetCurrentAmmoAttribute())
		.Remove(AmmoChangedHandle);
	
	ASC->GetGameplayAttributeValueChangeDelegate(UWeaponAttributeSet::GetMaxAmmoAttribute())
		.Remove(MaxAmmoChangedHandle);
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

void UPlayerStatusWidget::OnAmmoChanged(const FOnAttributeChangeData& Data)
{
	UpdateAmmo(Data.NewValue);
}

void UPlayerStatusWidget::OnMaxAmmoChanged(const FOnAttributeChangeData& Data)
{
     UpdateMaxAmmo(Data.NewValue);
}

// 퍼센트 계산
void UPlayerStatusWidget::UpdateHealth(float Current)
{
	const float Percent = (MaxHealth > 0.f) ? FMath::Clamp(Current / MaxHealth, 0.f, 1.f) : 0.f;
	if (PB_Health) PB_Health->SetPercent(Percent);
	
	if (TXT_Health)
	{
		const int32 CurInt = FMath::RoundToInt(Current); // 정수로 반올림
		TXT_Health->SetText(FText::AsNumber(CurInt)); // Text로 변환
	}
}

void UPlayerStatusWidget::UpdateStamina(float Current)
{
	const float Percent = (MaxStamina > 0.f) ? FMath::Clamp(Current / MaxStamina, 0.f, 1.f) : 0.f;
	if (PB_Stamina) PB_Stamina->SetPercent(Percent);
}

void UPlayerStatusWidget::UpdateAmmo(float Current)
{
	if (TXT_Ammo)
	{
		const int32 Cur = FMath::RoundToInt(Current);
		TXT_Ammo->SetText(FText::AsNumber(Cur));
	}
}

// MaxAmmo도 표시할 거면
void UPlayerStatusWidget::UpdateMaxAmmo(float Max)
{
	if (TXT_MaxAmmo)
	{
		const int32 M = FMath::RoundToInt(Max);
		TXT_MaxAmmo->SetText(FText::AsNumber(M));
	}
}
