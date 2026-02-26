#include "Items/BaseItem.h"

ABaseItem::ABaseItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic")); // 충돌 설정
	Mesh->SetSimulatePhysics(true); // 물리 시뮬레이션
}

