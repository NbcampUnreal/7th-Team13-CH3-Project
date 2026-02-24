// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h" // GameplayTag
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

// 전방 선언
class UWeaponAttributeSet;
class UWeaponDataAsset;
class USkeletalMeshComponent;
class UGameplayEffect;

UCLASS()
class FINALMINUTES_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

protected:
	virtual void BeginPlay() override;

public:
	/** 외부(Pawn 등)에서 무기 생성을 요청할 때 호출하는 진입점 */
	UFUNCTION(BlueprintCallable)
	void InitializeWeapon(FGameplayTag InWeaponTag, AActor* InOwner);

	void InitializeAttributes();

	/** 현재 무기에 로드된 정적 데이터 에셋을 반환합니다. */
	FORCEINLINE UWeaponDataAsset* GetCurrentDataAsset() const { return CurrentDataAsset; }

	/** 무기 비주얼을 담당하는 SkeletalMesh 컴포넌트를 반환합니다. */
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

protected:
	/** 메시 로딩 완료 후 호출될 콜백 */
	virtual void OnWeaponMeshLoaded(FGameplayTag InWeaponTag);

	/** 실제 캐릭터 소켓에 부착하는 로직 */
	void AttachToCharacter();

	/** 실제 캐릭터 소켓에 해제하는 로직 */
	void DetachFromCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(Transient)
	TObjectPtr<UWeaponDataAsset> CurrentDataAsset;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> WeaponOwner;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> InitStatEffectClass; // 무기 스탯 초기화용 GE 클래스

	FActiveGameplayEffectHandle WeaponStatEffectHandle; // 나중에 무기 해제 시 제거하기 위해 보관
};
