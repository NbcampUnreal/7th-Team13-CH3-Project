#include "FinalMinutes/Public/Subsystems/SaveSubsystem.h"
#include "AbilitySystem/Attributes/CharacterAttributeSet.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Core/FinalMinutesSaveGame.h"
#include "FinalMinutes/Public/Framework/FinalMinutesGameState.h"
#include "FinalMinutes/Public/Character/Player/PlayerCharacter.h"
#include "FinalMinutes/Public/Character/Components/CombatComponent.h"
#include "FinalMinutes/Public/Items/Weapons/WeaponDataAsset.h"
#include "FinalMinutes/Public/Items/Weapons/WeaponBase.h"
#include "Framework/FinalMinutesGameMode.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/PlayerController.h"


bool USaveSubsystem::bIsLoadingGame = false;
void USaveSubsystem::SaveGameData(int32 CurrentKillCount, float SurviveTime, FString SlotName)
{
	if (SlotName.IsEmpty())
	{
		SlotName = (CurrentSlotName.IsEmpty()) ? TEXT("SaveSlot_01") : CurrentSlotName;
	}
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
	
	if (CombatComp)
	{
		//마지막에 든 무기 태그 저장
		if (CombatComp->GetActiveWeapon() && CombatComp->GetActiveWeapon()->GetCurrentDataAsset())
		{
			SaveObject->LastEquipWeapon = CombatComp->GetActiveWeapon()->GetCurrentDataAsset()->WeaponData.WeaponTag;
		}
		
		AWeaponBase* PrimaryWeap = CombatComp->GetWeaponBySlot(EWeaponSlot::Primary);
		if (PrimaryWeap && PrimaryWeap->GetCurrentDataAsset())
		{
			SaveObject->PrimaryWeaponTag = PrimaryWeap->GetCurrentDataAsset()->WeaponData.WeaponTag;
		}
		
		AWeaponBase* SecondaryWeap = CombatComp->GetWeaponBySlot(EWeaponSlot::Secondary);
		if (SecondaryWeap && SecondaryWeap->GetCurrentDataAsset())
		{
			SaveObject->SecondaryWeaponTag = SecondaryWeap->GetCurrentDataAsset()->WeaponData.WeaponTag;
		}
		
		
		for (auto& Elem : CombatComp->GetSpawnedWeapons()) // SpawnedWeapons 맵 전체 순회
		{
			FGameplayTag WeaponTag = Elem.Key;
			AWeaponBase* WeaponInstance = Elem.Value;

			if (WeaponInstance)
			{
				int32 AmmoToSave = 0;

				// (A) 지금 손에 들고 있는 무기라면 GAS(실시간 수치)에서 가져옴
				if (WeaponInstance == CombatComp->GetActiveWeapon())
				{
					if (const UWeaponAttributeSet* WAS = ASC->GetSet<UWeaponAttributeSet>())
					{
						AmmoToSave = FMath::RoundToInt(WAS->GetCurrentAmmo());
					}
				}
				// (B) 등에 메고 있거나 보관 중인 무기라면 액터 변수에서 가져옴
				else
				{
					AmmoToSave = WeaponInstance->CurrentAmmoCount;
				}

				// 모든 무기의 탄약 정보를 장부에 기록!
				SaveObject->WeaponAmmoMap.Add(WeaponTag, AmmoToSave);
            
				UE_LOG(LogTemp, Log, TEXT("💾 [세이브] %s : %d발 저장 완료"), *WeaponTag.ToString(), AmmoToSave);
			}
		}
	}
    
	//무기 여러개 저장하는 로직 만들기
	if (UInventoryComponent* InvComp = Player->FindComponentByClass<UInventoryComponent>())
	{
		SaveObject->SavedInventory = InvComp->Items;
	}
       
	//로컬에 저장
	UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, 0);
}

void USaveSubsystem::LoadGameData(FString SlotName)
{
    if (SlotName.IsEmpty()) SlotName = CurrentSlotName;
    if (SlotName.IsEmpty()) SlotName = TEXT("SaveSlot_01");

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogTemp, Error, TEXT("!!! [로드 실패] 파일이 존재하지 않음: %s"), *SlotName);
        return;
    }

    UFinalMinutesSaveGame* LoadObject =
        Cast<UFinalMinutesSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (!LoadObject) return;

    AFinalMinutesGameState* GS = GetWorld()->GetGameState<AFinalMinutesGameState>();
    if (GS)
    {
        GS->SetLoadedData(LoadObject->TotalKillCount, LoadObject->BestSurviveTime);
        GS->bIsGameStarted = true;
        GS->GameTime = LoadObject->BestSurviveTime;
    }

    if (AFinalMinutesGameMode* GM = Cast<AFinalMinutesGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        GM->AdjustTimerAfterLoad(LoadObject->BestSurviveTime);
    }

    APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player) return;

    if (!LoadObject->PlayerLocation.IsNearlyZero())
    {
        Player->SetActorLocation(LoadObject->PlayerLocation, false, nullptr, ETeleportType::TeleportPhysics);
        Player->SetActorRotation(LoadObject->PlayerRotation, ETeleportType::TeleportPhysics);

        if (AController* PC = Player->GetController())
        {
            PC->SetControlRotation(LoadObject->PlayerRotation);
        }
    }

    UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
    if (ASC)
    {
        if (UCharacterAttributeSet* CAS = const_cast<UCharacterAttributeSet*>(ASC->GetSet<UCharacterAttributeSet>()))
        {
            CAS->SetHealth(LoadObject->CurrentHealth);
            CAS->SetStamina(LoadObject->CurrentStamina);
            ASC->ForceReplication();
        }
    }

    if (UInventoryComponent* InvComp = Player->FindComponentByClass<UInventoryComponent>())
    {
        InvComp->Items = LoadObject->SavedInventory;
        InvComp->OnInventoryUpdated.Broadcast();
    }

    // 무기 로드 순서 수정
    UCombatComponent* CombatComp = Player->GetCombatComponent();
    if (CombatComp)
    {
        // 반드시 EquipWeapon 호출 전에 PendingLoadedAmmoMap 주입
        CombatComp->SetPendingLoadedAmmoMap(LoadObject->WeaponAmmoMap);
    	UE_LOG(LogTemp, Error, TEXT("👤 [SaveSubsystem] 장부 준 곳 주소: %p, 개수: %d"), CombatComp, LoadObject->WeaponAmmoMap.Num());
    	UE_LOG(LogTemp,
    		Warning,
    		TEXT("📦 [SaveSubsystem] 장부 전달 완료! 개수: %d"),
    		LoadObject->WeaponAmmoMap.Num());

        // 슬롯 복구
        if (LoadObject->PrimaryWeaponTag.IsValid())
        {
            CombatComp->EquipWeapon(LoadObject->PrimaryWeaponTag);
        }

        if (LoadObject->SecondaryWeaponTag.IsValid())
        {
            CombatComp->EquipWeapon(LoadObject->SecondaryWeaponTag);
        }

        // 마지막으로 들고 있던 무기
        if (LoadObject->LastEquipWeapon.IsValid())
        {
            CombatComp->EquipWeapon(LoadObject->LastEquipWeapon);
        }
    }

    // 마우스 커서 끄기용 타이머
    FTimerHandle Handle;
    GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
        {
            PC->SetShowMouseCursor(false);
            PC->SetInputMode(FInputModeGameOnly());
        }
    }, 0.1f, false);
}