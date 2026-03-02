#include "Items/BaseItem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

ABaseItem::ABaseItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic")); // 충돌 설정
	Mesh->SetSimulatePhysics(true); // 물리 시뮬레이션
}

void ABaseItem::BeginPlay()
{
	Super::BeginPlay();

	// BP에 달린 WidgetComponent 잡기 + 기본 숨김
	PromptWidget = FindComponentByClass<UWidgetComponent>();
	if (PromptWidget)
	{
		PromptWidget->SetVisibility(false, true);
	}
}

void ABaseItem::SetOutline(bool bEnable)
{
	TArray<UStaticMeshComponent*> Meshes;
	GetComponents<UStaticMeshComponent>(Meshes);

	for (UStaticMeshComponent* SM : Meshes)
	{
		if (!SM) continue;

		SM->SetRenderCustomDepth(bEnable);
		if (bEnable)
		{
			SM->SetCustomDepthStencilValue(OutlineStencilValue);
		}
	}
}

void ABaseItem::SetPromptVisible(bool bVisible)
{
	// ✅ 필요할 때마다 찾아서 NULL 방지
	if (!PromptWidget)
	{
		PromptWidget = FindComponentByClass<UWidgetComponent>();
		if (!PromptWidget) return;
	}

	PromptWidget->SetVisibility(bVisible, true);

	if (bVisible)
	{
		// ✅ 보일 때만 회전 업데이트 (아이템 Tick 필요 없음)
		GetWorldTimerManager().SetTimer(
			PromptFacingTimer,
			this,
			&ABaseItem::UpdatePromptFacing,
			0.03f,   // 0.03~0.05 추천
			true
		);

		UpdatePromptFacing(); // 즉시 1회 반영
	}
	else
	{
		// ✅ 숨기면 타이머 중지
		GetWorldTimerManager().ClearTimer(PromptFacingTimer);
	}
}

void ABaseItem::UpdatePromptFacing()
{
	if (!PromptWidget || !PromptWidget->IsVisible()) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector WidgetLoc = PromptWidget->GetComponentLocation();

	// ✅ 위젯만 카메라 방향으로 회전
	FRotator LookAt = (CamLoc - WidgetLoc).Rotation();

	// ✅ 기울어짐 싫으면 Yaw만 사용
	LookAt.Pitch = 0.f;
	LookAt.Roll  = 0.f;

	PromptWidget->SetWorldRotation(LookAt);
}

