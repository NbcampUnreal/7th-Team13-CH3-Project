#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "FinalMinutesSaveGame.generated.h"


UCLASS()
class FINALMINUTES_API UFinalMinutesSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	//저장할 슬롯 이름
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData")
	FString SaveSlotName = TEXT("DefaultSlot");
	
	//세이브 파일 인덱스
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 SaveIndex = 0;
	
	//로컬에 저장될 데이터 누적 킬카운트
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData", SaveGame)
	int32 TotalKillCount =0;
	
	//최고 생존 시간
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite, Category = "SaveData", SaveGame)
	float BestSurviveTime =0.0f;
	
	//플레이어가 마지막으로 들고 있던 무기
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData", SaveGame)
	FGameplayTag LastEquipWeapon;
	
	//무기 여러개 장착시 태그 여러개 목록 저장하는 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveData", SaveGame)
	TArray<FGameplayTag> InventoryWeapons;
	
	//플레이어가 있던 마지막 위치 (세이브 로드시 스폰될 위치)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData", SaveGame)
	FVector PlayerLocation = FVector::ZeroVector;
	
	//플레이어가 마지막으로 바라보던 방향
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData", SaveGame)
	FRotator PlayerRotation = FRotator::ZeroRotator;
	
	//플레이어 마지막 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData", SaveGame)
	float CurrentHealth = 100.0f;
	
	//플레이어 마지막 스테미나
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData", SaveGame)
	float CurrentStamina = 100.0f;

};
