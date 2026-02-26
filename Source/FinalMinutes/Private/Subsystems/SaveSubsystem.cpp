#include "FinalMinutes/Public/Subsystems/SaveSubsystem.h"

#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
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
	
	//플레이어가 죽었거나 없으면
	if (!Player)
	{
		UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, 0);
		return;
	}
	
	//플레이어가 존재하면 위치/ 회전 기록
	SaveObject->PlayerLocation = Player->GetActorLocation();
	SaveObject->PlayerRotation = Player->GetActorRotation();
	
	//플레이어 체력/스테미나 기록
	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
	if (ASC)
	{
		if (const UCharacterAttributeSet* CAS = ASC->GetSet<UCharacterAttributeSet>())
		{
			SaveObject->CurrentHealth = CAS->GetHealth();
			SaveObject->CurrentStamina = CAS->GetStamina();
		}
	}
	
	//플레이어 무기 기록
	UCombatComponent* CombatComp = Player->GetCombatComponent();
	//무기 컴포넌트 가 없다면 아마 여까지 하고 저장
	if (!CombatComp)
	{
		UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, 0);
		return;
	}
	
	if (CombatComp->GetActiveWeapon() && CombatComp->GetActiveWeapon()->GetCurrentDataAsset())
	{
		//마지막에 장착했던 무기를 저장
		SaveObject->LastEquipWeapon = CombatComp->GetActiveWeapon()->GetCurrentDataAsset()->WeaponData.WeaponTag;
	}
	
	/*
	//여긴 총 여러개 완성?하면 추가해도 될듯
	//장착되지않은 주무기,보조무기 등 저장하는거
	SaveObject->InventoryWeapons.Empty();
	
	 //무기 컴포넌트를 가지고 있는 인벤 태그를 하나씩 Tag에 넣음
	 for(FGameplayTag Tag : CombatComp-> 인벤토리태그s)
	 {
	   SaveObject->InventoryWeapons.Add(Tag);
	 }
	 */
	
	//로컬에 저장
	UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, 0);
	}

void USaveSubsystem::LoadGameData(FString SlotName)
{
	//파일이 없으면 여기서 바로 종료 (Early Return)
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0)) return; 
	
	//로드 실패 시 바로 종료
	UFinalMinutesSaveGame* LoadObject = Cast<UFinalMinutesSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!LoadObject) return; 

	// UI 갱신 
	AFinalMinutesGameState* GS = GetWorld()->GetGameState<AFinalMinutesGameState>();
	if (GS)
	{
		GS->SetLoadedData(LoadObject->TotalKillCount, LoadObject->BestSurviveTime);
	}
	
	//플레이어나 컴포넌트를 못 찾으면 종료 
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player) return;
	
	//위치 방향 
	//캐릭터가 스폰되지 않았거나 초기화면 0,0,0이니까 그게 아니면
	if (!LoadObject->PlayerLocation.IsNearlyZero())
	{
		//캐릭터를 저장된 위치/보고있던 방향으로 맞추겠다
		Player->SetActorLocation(LoadObject->PlayerLocation);
		Player->SetActorRotation(LoadObject->PlayerRotation);
	}
	
	//체력,스테미나
	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
	if (ASC)
	{
		if (UCharacterAttributeSet* CAS = const_cast<UCharacterAttributeSet*>(ASC->GetSet<UCharacterAttributeSet>()))
		{
			CAS->SetHealth(LoadObject->CurrentHealth);
			CAS->SetStamina(LoadObject->CurrentStamina);
			//UI에 바로 반영되도록 강제로 업데이트 해줌
			ASC->ForceReplication();
		}
	}
		
	//세이브된 무기 태그가 유효하지 않으면 종료
	if (!LoadObject->LastEquipWeapon.IsValid()) return; 
	
	/*
	//저장했던 인벤토리 무기를 다시 인벤에 넣어버리기
	//무기 컴포넌트를 가지고 있는 인벤 태그를 하나씩 Tag에 넣음
	 for(FGameplayTag SaveTag : LoadObject-> InventoryWeapons)
	 {
	 //무기 컴포넌트에 무기를 추가하는 함수가 있다면 호출
	   Player->GetCombatComponent()->무기를 추가하는 함수 아마 AddWeaponToInventory(SaveTag);
	 }
	 */
	
	//무기 장착
	Player->GetCombatComponent()->EquipWeapon(LoadObject->LastEquipWeapon);
}