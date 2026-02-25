#include "FinalMinutes/Public/Subsystems/SaveSubsystem.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Core/FinalMinutesSaveGame.h"
#include "FinalMinutes/Public/Framework/FinalMinutesGameState.h"
#include "FinalMinutes/Public/Character/Player/PlayerCharacter.h"
#include "FinalMinutes/Public/Character/Components/CombatComponent.h"
#include "FinalMinutes/Public/Items/Weapons/WeaponDataAsset.h"
#include "FinalMinutes/Public/Items/Weapons/WeaponBase.h"
#include "Framework/FinalMinutesGameMode.h"

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
	
	if (CombatComp->GetCurrentWeapon() && CombatComp->GetCurrentWeapon()->GetCurrentDataAsset())
	{
		//마지막에 장착했던 무기를 저장
		SaveObject->LastEquipWeapon = CombatComp->GetCurrentWeapon()->GetCurrentDataAsset()->WeaponData.WeaponTag;
	}
	
	//무기 여러개 저장하는 로직 만들기(인벤토리 기능 만들어지면)
	
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
	//게임모드 - 남은시간 다시 계산하기
	if (AFinalMinutesGameMode* GM = Cast<AFinalMinutesGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		// 세이브 파일에 적힌 시간 넘기기
		GM->AdjustTimerAfterLoad(LoadObject->BestSurviveTime);
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
	
	//무기 여러개 로드하는 로직 만들기 (인벤토리 기능 만들어 지면)
	
	//무기 장착
	Player->GetCombatComponent()->EquipWeapon(LoadObject->LastEquipWeapon);
}