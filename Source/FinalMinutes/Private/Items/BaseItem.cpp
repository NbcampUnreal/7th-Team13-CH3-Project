#include "Items/BaseItem.h"
#include "Components/StaticMeshComponent.h"

ABaseItem::ABaseItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic")); // 충돌 설정
	Mesh->SetSimulatePhysics(true); // 물리 시뮬레이션
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

