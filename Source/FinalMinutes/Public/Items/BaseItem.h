#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseItem.generated.h"

class UStaticMeshComponent;

UCLASS()
class FINALMINUTES_API ABaseItem : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseItem();

protected:
	// 아이템의 외형을 담당할 변수를 만듭니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	UStaticMeshComponent* Mesh;

public:	
	// ItemID를 저장할 변수, 내가 누구인지 알려주는 값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FName ItemID;
	
	UFUNCTION(BlueprintCallable, Category="Outline")
	void SetOutline(bool bEnable);

	UPROPERTY(EditDefaultsOnly, Category="Outline")
	int32 OutlineStencilValue = 1;
};
