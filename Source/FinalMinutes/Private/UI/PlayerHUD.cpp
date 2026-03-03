#include "UI/PlayerHUD.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Character/Player/PlayerCharacter.h"
#include "UI/PlayerStatusWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;
	
	if (MainHUDWidgetClass && !MainHUDWidget)
	{
		MainHUDWidget = CreateWidget<UPlayerStatusWidget>(PC, MainHUDWidgetClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();
		}
	}

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
	if (!NewPawn) return;

	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(NewPawn);
	if (!PlayerChar) return;

	UAbilitySystemComponent* ASC = PlayerChar->GetAbilitySystemComponent();
	if (!ASC) return;

	CreateAndInit(ASC);
}

void APlayerHUD::CreateAndInit(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;
	if (!MainHUDWidgetClass) return;
	
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	if (!MainHUDWidget)
	{
		MainHUDWidget = CreateWidget<UPlayerStatusWidget>(PC, MainHUDWidgetClass);
		if (!MainHUDWidget) return;

		MainHUDWidget->AddToViewport();
	}

	// Pawn/ASC가 바뀔 때마다 항상 다시 연결
	MainHUDWidget->InitWithASC(ASC);
}

UPlayerStatusWidget* APlayerHUD::GetMainHUDWidget()
{
	if (!MainHUDWidget)
	{
		APlayerController* PC = GetOwningPlayerController();
		if (PC && MainHUDWidgetClass)
		{
			MainHUDWidget = CreateWidget<UPlayerStatusWidget>(PC, MainHUDWidgetClass);
			if (MainHUDWidget)
			{
				MainHUDWidget->AddToViewport();
			}
		}
	}
	return MainHUDWidget;
}

// UI창 띄우기
void APlayerHUD::ShowResultUI(TSubclassOf<UUserWidget> WidgetClass)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !WidgetClass) return;

	if (MainHUDWidget) MainHUDWidget->SetVisibility(ESlateVisibility::Collapsed);

	if (!ResultWidget)
	{
		ResultWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
		if (ResultWidget) ResultWidget->AddToViewport(200);
	}

	PC->SetShowMouseCursor(true);

	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(ResultWidget->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(Mode);

	UGameplayStatics::SetGamePaused(this, true);
}

// 게임 오버 화면
void APlayerHUD::ShowGameOverUI()
{
	ShowResultUI(GameOverWidgetClass);
}

// 게임 클리어 화면
void APlayerHUD::ShowGameClearUI()
{
	ShowResultUI(GameClearWidgetClass);
}