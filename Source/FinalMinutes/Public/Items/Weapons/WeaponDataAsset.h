#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "WeaponDataAsset.generated.h"


USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	//무기 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Tag")
	FGameplayTag WeaponTag;

	//무기 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Info")
	FString WeaponName;

	//무기 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Info")
	float WeaponDamage;
	
	//데이터테이블에서 사운드, 메쉬, 소리, 모션들, 총알 속도등 - 에셋, 무기메쉬 위치를 포함시키는 방향 으로
	
	//메쉬
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Mesh")
	TSoftObjectPtr<USkeletalMesh> WeaponMesh;
	
	//메쉬 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Mesh")
	FTransform WeaponEquipTransform; //WeaponHand
	
	//애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Anim")
	TSoftObjectPtr<UAnimSequence> FireAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Anim")
	TSoftObjectPtr<UAnimSequence> ReloadAnim;
	
	//사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sound")
	TSoftObjectPtr<USoundBase> FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sound")
	TSoftObjectPtr<USoundBase> ReloadSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Sound")
	TSoftObjectPtr<USoundBase> EmptySound;
	
	//총알 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Info")
	float BulletSpeed;
	
	//이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effect")
	TSoftObjectPtr<class UNiagaraSystem> FireEffect;
	
	// 머즐 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effect")
	FName MuzzleName = FName("MuzzleFlash");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effect")
	FTransform MuzzleTransform;
};