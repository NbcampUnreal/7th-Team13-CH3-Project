#include "FinalMinutes/Public/Subsystems/SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Core/FinalMinutesSaveGame.h"
#include "FinalMinutes/Public/Framework/FinalMinutesGameState.h"
#include "FinalMinutes/Public/Character/Player/PlayerCharacter.h"
#include "FinalMinutes/Public/Character/Components/CombatComponent.h"
#include "FinalMinutes/Public/Items/Weapons/WeaponDataAsset.h"
#include "FinalMinutes/Public/Items/Weapons/WeaponBase.h"

void USaveSubsystem::SaveGameData(int32 CurrentKillCount, float SurviveTime, FString SlotName)
{
	//SaveObject 객체 생성하고 SaveGame 포인터 형식으로 캐스팅해서 저장
	//SaveGame 인스턴스 생성 후 형변환 
	UFinalMinutesSaveGame* SaveObject = Cast<UFinalMinutesSaveGame>(UGameplayStatics::CreateSaveGameObject(UFinalMinutesSaveGame::StaticClass()));
    
	if (!SaveObject) return;
	
	// 데이터 기록 - 킬카운트 ,생존시간
	SaveObject->TotalKillCount = CurrentKillCount;
	SaveObject->BestSurviveTime = SurviveTime; 
        
	// 현재 플레이어의 무기 태그 추출
	//로컬 플레이어 가져오기 Player0
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	//플레이어가 있고 장착된 무기가 있을 때
	if (Player && Player->GetCombatComponent() && Player->GetCombatComponent()->GetCurrentWeapon())
	{
		//현재 무기에서 DataAsset을 가져와 WeaponAsset 포인터에 저장
		UWeaponDataAsset* WeaponAsset = Player->GetCombatComponent()->GetCurrentWeapon()->GetCurrentDataAsset();
		if (WeaponAsset)
		{
			//유호하면 SaveObject에 마지막으로 장착한 무기의 태그를 기록
			SaveObject->LastEquipWeapon = WeaponAsset->WeaponData.WeaponTag;
		}
	}
	//로컬에 저장
	UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, 0);
	}

void USaveSubsystem::LoadGameData(FString SlotName)
{
	// 1. 파일이 없으면 여기서 바로 종료 (Early Return)
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0)) return; 
	
	// 2. 로드 실패 시 바로 종료
	UFinalMinutesSaveGame* LoadObj = Cast<UFinalMinutesSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!LoadObj) return; 

	// 3. UI 갱신 
	AFinalMinutesGameState* GS = GetWorld()->GetGameState<AFinalMinutesGameState>();
	if (GS)
	{
		GS->SetLoadedData(LoadObj->TotalKillCount, LoadObj->BestSurviveTime);
	}
    
	// 4. 무기 복구
	// 세이브된 무기 태그가 유효하지 않으면 종료
	if (!LoadObj->LastEquipWeapon.IsValid()) return; 

	// 플레이어나 컴포넌트를 못 찾으면 종료 
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player || !Player->GetCombatComponent()) return;

	//무기 장착
	Player->GetCombatComponent()->EquipWeapon(LoadObj->LastEquipWeapon);
}