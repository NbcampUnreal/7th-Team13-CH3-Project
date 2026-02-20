#include "UI/PlayerHUD.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Character/Player/PlayerCharacter.h"
#include "UI/PlayerStatusWidget.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	// 이미 Pawn이 있으면 바로 위젯 생성/ASC 연결
	TryCreateWidgetFromPawn(PC->GetPawn());

	// Pawn이 나중에 바뀌면(스폰/리스폰 등) 이벤트로 감지해서 다시 시도
	PC->GetOnNewPawnNotifier().AddUObject(this, &APlayerHUD::OnPawnChanged);
}

void APlayerHUD::OnPawnChanged(APawn* NewPawn)
{
	// 새 Pawn으로 UI 연결 시도
	TryCreateWidgetFromPawn(NewPawn);
}

void APlayerHUD::TryCreateWidgetFromPawn(APawn* NewPawn)
{
	if (StatusWidget) return;          // 중복 생성 방지
	if (!StatusWidgetClass) return;
	if (!NewPawn) return;

	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(NewPawn);
	if (!PlayerChar) return;

	UAbilitySystemComponent* ASC = PlayerChar->GetAbilitySystemComponent();
	if (!ASC) return;

	CreateAndInit(ASC);
}

void APlayerHUD::CreateAndInit(UAbilitySystemComponent* ASC)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	StatusWidget = CreateWidget<UPlayerStatusWidget>(PC, StatusWidgetClass);
	if (!StatusWidget) return;

	StatusWidget->AddToViewport();
	StatusWidget->InitWithASC(ASC);
}
