// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h" // GameplayTag
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

// 전방 선언
class UWeaponAttributeSet;
class UWeaponDataAsset;
class USkeletalMeshComponent;

UCLASS()
class FINALMINUTES_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

protected:
	virtual void BeginPlay() override;

public:
	/** 무기 장착/해제 함수 */
	void ActivateWeapon();
	void DeactivateWeapon();
	
	/** 데이터 에셋을 통한 초기화 함수 */
    void InitializeWeapon(UWeaponDataAsset* InDataAsset, UWeaponAttributeSet* InAttributes);
	
private:
	/** 비동기 로드 완료 시 호출될 콜백 함수 */
    void OnWeaponMeshLoaded() const;
private:
	/** 실제 무기 외형을 담당하는 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category = "Weapon|Visual")
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;
	
	/** * 무기 고유 식별자
     * FString 대신 GameplayTag를 사용하여 조건 검사 속도와 확장성을 확보
     */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Core")
    FGameplayTag WeaponTag;
	
	/** * 무기의 설계도 참조
     * 기본 정보: 무기 이름, 무기 태그
     * 에셋 참조: 스켈레탈 메시, 사운드 큐(발사/장전), 투사체 클래스, 애니메이션 몽타주
     * 수치 데이터: 기본 탄창 크기, 기본 공격력, 기본 연사 속도, 소음 크기, 기본 반동 값, 최대 탄퍼짐 범위
     * 무기 소켓 정보: 무기 ↔ 캐릭터 소켓, 총알 ↔ 총구 소켓, 탄피 ↔ 배출구 소켓
     */
    UPROPERTY(VisibleAnywhere, Category = "Weapon|Data")
    TObjectPtr<const UWeaponDataAsset> WeaponData;
	
	/** 무기 실시간 속성
	 * 수치 데이터: 현재/최대 탄창 수, 현재 공격력, 현재 연사 속도, 현재 재장전 속도, 현재 총알 속도, 소음 배율, 유효 사거리, 조준 속도, 현재 탄퍼짐 계수
	 */
    UPROPERTY(VisibleAnywhere, Category = "Weapon|GAS")
    TObjectPtr<UWeaponAttributeSet> WeaponAttributes;
};
