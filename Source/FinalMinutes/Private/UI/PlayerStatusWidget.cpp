#include "UI/PlayerStatusWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Character/Player/PlayerCharacter.h"
#include "Character/Components/CombatComponent.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

#include "Components/InventoryComponent.h"
#include "Items/Weapons/WeaponBase.h"
#include "Items/Weapons/WeaponDataAsset.h"

void UPlayerStatusWidget::InitWithASC(UAbilitySystemComponent* InASC)
{
	// 기존 바인딩 해제 후 다시 연결 (중복 호출 방지)
	UnbindInventoryCallbacks();
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
	}

	// Max는 고정이므로 1회만 저장
	MaxHealth  = FMath::Max(CAS->GetMaxHealth(), 1.f);
	MaxStamina = FMath::Max(CAS->GetMaxStamina(), 1.f);

	// UI 초기값 즉시 반영
	UpdateHealth(CAS->GetHealth());
	UpdateStamina(CAS->GetStamina());

	BindCallbacks(); // 델리게이트 바인딩
	BindCombatCallbacks(); // 무기 변경 이벤트 바인딩
	
	BindInventoryCallbacks();
	UpdateReserveAmmo();
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
	// 혹시 아직 인벤 바인딩 안됐으면 지금이라도 시도
	if (!BoundInventoryComp)
	{
		BindInventoryCallbacks();
	}
	UpdateReserveAmmo();  
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
}

// 위젯이 사라질 때 델리게이트 연결 해제
void UPlayerStatusWidget::NativeDestruct()
{
	UnbindInventoryCallbacks();
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

void UPlayerStatusWidget::ShowDamageNumber(int32 Damage)
{
	if (!Canvas_DamageLayer || !DamagePopupClass) return;
	if (Damage <= 0) return;

	UUserWidget* Popup = CreateWidget<UUserWidget>(GetWorld(), DamagePopupClass);
	if (!Popup) return;

	UCanvasPanelSlot* CanvasSlot = Canvas_DamageLayer->AddChildToCanvas(Popup);
	if (!CanvasSlot) return;
	
	CanvasSlot->SetPosition(FVector2D(0.f, 0.f));
	CanvasSlot->SetAutoSize(true);
	CanvasSlot->SetZOrder(999);

	// BP 함수 InitDamage(int) 호출
	static const FName InitFuncName(TEXT("InitDamage"));
	if (UFunction* Func = Popup->FindFunction(InitFuncName))
	{
		struct FInitDamageParams { int32 Damage; };
		FInitDamageParams Params{ Damage };
		Popup->ProcessEvent(Func, &Params);
	}
}

void UPlayerStatusWidget::ShowKillPlusOne()
{
	if (!Canvas_KillLayer || !KillPopupClass) return;

	UUserWidget* Popup = CreateWidget<UUserWidget>(GetWorld(), KillPopupClass);
	if (!Popup) return;

	UCanvasPanelSlot* CanvasSlot = Canvas_KillLayer->AddChildToCanvas(Popup);
	if (!CanvasSlot) return;

	CanvasSlot->SetPosition(FVector2D(0.f, 0.f));
	CanvasSlot->SetAutoSize(true);
	CanvasSlot->SetZOrder(999);
}

void UPlayerStatusWidget::ShowGameStartMessage()
{
	if (!Canvas_MessageLayer || !GameMessagePopupClass) return;

	UUserWidget* Popup = CreateWidget<UUserWidget>(GetWorld(), GameMessagePopupClass);
	if (!Popup) return;

	UCanvasPanelSlot* CanvasSlot = Canvas_MessageLayer->AddChildToCanvas(Popup);
	if (!CanvasSlot) return;

	CanvasSlot->SetPosition(FVector2D(0.f, 0.f));
	CanvasSlot->SetAutoSize(true);
	CanvasSlot->SetZOrder(999);
}

void UPlayerStatusWidget::BindInventoryCallbacks()
{
	if (!ASC) return;

	AActor* Avatar = ASC->GetAvatarActor();
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Avatar);
	if (!PlayerChar) return;

	UInventoryComponent* Inv = PlayerChar->FindComponentByClass<UInventoryComponent>();
	if (!Inv) return;

	if (BoundInventoryComp == Inv) return;

	if (BoundInventoryComp)
	{
		BoundInventoryComp->OnInventoryUpdated.RemoveAll(this);
		BoundInventoryComp = nullptr;
	}

	BoundInventoryComp = Inv;
	BoundInventoryComp->OnInventoryUpdated.AddDynamic(this, &UPlayerStatusWidget::HandleInventoryUpdated);
}

void UPlayerStatusWidget::UnbindInventoryCallbacks()
{
	if (BoundInventoryComp)
	{
		BoundInventoryComp->OnInventoryUpdated.RemoveAll(this);
		BoundInventoryComp = nullptr;
	}
}

void UPlayerStatusWidget::HandleInventoryUpdated()
{
	UpdateReserveAmmo();
}

FName UPlayerStatusWidget::ResolveAmmoItemIDFromActiveWeapon() const
{
	if (!ASC) return NAME_None;

	AActor* Avatar = ASC->GetAvatarActor();
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Avatar);
	if (!PlayerChar) return NAME_None;

	UCombatComponent* CC = PlayerChar->FindComponentByClass<UCombatComponent>();
	AWeaponBase* Weapon = CC ? CC->GetActiveWeapon() : nullptr;
	if (!Weapon) return NAME_None;

	const UWeaponDataAsset* DA = Weapon->GetCurrentDataAsset();
	if (!DA) return NAME_None;

	return DA->WeaponData.AmmoItemID;
}

void UPlayerStatusWidget::UpdateReserveAmmo()
{
	if (!TXT_MaxAmmo) return;

	// 현재 무기 기준 탄약 ID 갱신
	CurrentAmmoItemID = ResolveAmmoItemIDFromActiveWeapon();

	const int32 Reserve =
		(BoundInventoryComp && !CurrentAmmoItemID.IsNone())
		? BoundInventoryComp->GetItemQuantity(CurrentAmmoItemID)
		: 0;

	TXT_MaxAmmo->SetText(FText::AsNumber(Reserve));
}