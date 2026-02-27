#include "UI/PlayerStatusWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Character/Player/PlayerCharacter.h"
#include "Character/Components/CombatComponent.h"

void UPlayerStatusWidget::InitWithASC(UAbilitySystemComponent* InASC)
{
	// 기존 바인딩 해제 후 다시 연결 (중복 호출 방지)
	UnbindCombatCallbacks();
	UnbindCallbacks();

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
		UpdateMaxAmmo(WAS->GetMaxAmmo());
	}

	// Max는 고정이므로 1회만 저장
	MaxHealth  = FMath::Max(CAS->GetMaxHealth(), 1.f);
	MaxStamina = FMath::Max(CAS->GetMaxStamina(), 1.f);

	// UI 초기값 즉시 반영
	UpdateHealth(CAS->GetHealth());
	UpdateStamina(CAS->GetStamina());

	BindCallbacks(); // 델리게이트 바인딩
	BindCombatCallbacks(); // 무기 변경 이벤트 바인딩
}
// CombatComponent 델리게이트 바인딩
void UPlayerStatusWidget::BindCombatCallbacks()
{
	if (!ASC) return;

	// ASC의 Avatar(캐릭터)를 통해 CombatComponent 찾기
	AActor* Avatar = ASC->GetAvatarActor();
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Avatar);
	if (!PlayerChar) return;

	UCombatComponent* CC = PlayerChar->FindComponentByClass<UCombatComponent>();
	if (!CC) return;

	// 이미 같은 컴포넌트에 바인딩 돼있으면 중복 방지
	if (BoundCombatComp == CC) return;

	// 다른 컴포넌트에 바인딩돼 있었다면 해제
	if (BoundCombatComp)
	{
		BoundCombatComp->OnActiveWeaponTagChanged.RemoveAll(this);
		BoundCombatComp = nullptr;
	}

	BoundCombatComp = CC;
	BoundCombatComp->OnActiveWeaponTagChanged.AddDynamic(
		this, &UPlayerStatusWidget::HandleActiveWeaponTagChanged
	);
}

// CombatComponent 델리게이트 해제
void UPlayerStatusWidget::UnbindCombatCallbacks()
{
	if (BoundCombatComp)
	{
		BoundCombatComp->OnActiveWeaponTagChanged.RemoveAll(this);
		BoundCombatComp = nullptr;
	}
}

// 무기 태그 변경 시 이벤트 수신 → BP 이벤트 호출
void UPlayerStatusWidget::HandleActiveWeaponTagChanged(FGameplayTag WeaponTag)
{
	OnActiveWeaponTagChanged(WeaponTag); // BP에서 아이콘 바꾸는 이벤트
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
	UnbindCombatCallbacks();
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

void UPlayerStatusWidget::UpdateMaxAmmo(float Max)
{
	if (TXT_MaxAmmo)
	{
		const int32 M = FMath::RoundToInt(Max);
		TXT_MaxAmmo->SetText(FText::AsNumber(M));
	}
}
