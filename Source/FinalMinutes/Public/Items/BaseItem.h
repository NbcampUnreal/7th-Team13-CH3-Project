#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseItem.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;

UCLASS()
class FINALMINUTES_API ABaseItem : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseItem();
	
	UFUNCTION(BlueprintCallable, Category="Outline")
	void SetOutline(bool bEnable);

	UFUNCTION(BlueprintCallable)
	void SetPromptVisible(bool bVisible);

protected:
	virtual void BeginPlay() override;
	
	// 아이템의 외형을 담당할 변수를 만듭니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	TObjectPtr<class UWidgetComponent> PromptWidget;

public:	
	// ItemID를 저장할 변수, 내가 누구인지 알려주는 값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FName ItemID;
	
	
	UPROPERTY(EditDefaultsOnly, Category="Outline")
	int32 OutlineStencilValue = 1;
	
private:
	// ✅ 위젯이 카메라를 바라보도록 갱신(아이템은 안 돌고 위젯만 돎)
	void UpdatePromptFacing();
	
	FTimerHandle PromptFacingTimer;
};
