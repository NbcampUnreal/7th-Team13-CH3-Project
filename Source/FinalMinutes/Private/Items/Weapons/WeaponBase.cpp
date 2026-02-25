#include "Items/Weapons/WeaponBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Items/Weapons/WeaponDataAsset.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/WeaponRegistrySubsystem.h"
#include "NiagaraFunctionLibrary.h"

AWeaponBase::AWeaponBase()
{
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;
    
    // 무기 기본 충돌 설정 (캐릭터와 부딪힘 방지)
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetSimulatePhysics(false);
}

void AWeaponBase::InitializeWeapon(FGameplayTag InWeaponTag, AActor* InOwner)
{
    /*WeaponOwner = InOwner;
    if (!WeaponOwner.IsValid() || !InWeaponTag.IsValid()) return;

    UWeaponRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UWeaponRegistrySubsystem>();
    CurrentDataAsset = Registry ? Registry->GetWeaponDataByTag(InWeaponTag) : nullptr;

    if (CurrentDataAsset)
    {
        TSoftObjectPtr<USkeletalMesh> MeshPtr = CurrentDataAsset->WeaponData.WeaponMesh;
        if (MeshPtr.IsPending())
        {
            FStreamableManager& Streamable = UAssetManager::Get().GetStreamableManager();
            Streamable.RequestAsyncLoad(MeshPtr.ToSoftObjectPath(), 
                FStreamableDelegate::CreateUObject(this, &AWeaponBase::OnWeaponMeshLoaded, InWeaponTag));
        }
        else
        {
            OnWeaponMeshLoaded(InWeaponTag);
        }
    }*/
    
    WeaponOwner = InOwner;
    UWeaponRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UWeaponRegistrySubsystem>();
    CurrentDataAsset = Registry ? Registry->GetWeaponDataByTag(InWeaponTag) : nullptr;

    if (CurrentDataAsset)
    {
        // 메시 로딩 로직 (비동기 등 기존 로직 유지)
        OnWeaponMeshLoaded(InWeaponTag);
    }
}

void AWeaponBase::OnWeaponMeshLoaded(FGameplayTag InWeaponTag)
{
    if (USkeletalMesh* LoadedMesh = CurrentDataAsset->WeaponData.WeaponMesh.Get())
    {
        WeaponMesh->SetSkeletalMesh(LoadedMesh);
        if (bIsActiveWeapon)
        {
            SetActorHiddenInGame(false);
            AttachToCharacter();
        }
        else
        {
            SetActorHiddenInGame(true);
        }
    }
}

void AWeaponBase::AttachToCharacter()
{
    /*IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(WeaponOwner.Get());
    if (!ASCHolder || !CurrentDataAsset) return;

    UAbilitySystemComponent* ASC = ASCHolder->GetAbilitySystemComponent();
    if (ASC)
    {
        // 무기 전용 태그 부여
        ASC->AddLooseGameplayTag(CurrentDataAsset->WeaponData.WeaponTag);
    }

    // 소켓 부착 로직
    USkeletalMeshComponent* ParentMesh = WeaponOwner->FindComponentByClass<USkeletalMeshComponent>();
    if (ParentMesh)
    {
        FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);
        AttachToComponent(ParentMesh, AttachRules, CurrentDataAsset->WeaponData.HandSocketName);
        WeaponMesh->SetRelativeTransform(CurrentDataAsset->WeaponData.WeaponMeshOffset);
    }*/
    
    IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(WeaponOwner.Get());
    if (ASCHolder && ASCHolder->GetAbilitySystemComponent())
    {
        // 장착 태그 부여 (중첩 방지를 위해 하나만 부여)
        ASCHolder->GetAbilitySystemComponent()->AddLooseGameplayTag(CurrentDataAsset->WeaponData.WeaponTag);
    }

    USkeletalMeshComponent* ParentMesh = WeaponOwner->FindComponentByClass<USkeletalMeshComponent>();
    if (ParentMesh)
    {
        AttachToComponent(ParentMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentDataAsset->WeaponData.HandSocketName);
        WeaponMesh->SetRelativeTransform(CurrentDataAsset->WeaponData.WeaponMeshOffset);
    }
}

void AWeaponBase::DetachFromCharacter()
{
    /*IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(WeaponOwner.Get());
    if (ASCHolder && CurrentDataAsset)
    {
        if (UAbilitySystemComponent* ASC = ASCHolder->GetAbilitySystemComponent())
        {
            ASC->RemoveLooseGameplayTag(CurrentDataAsset->WeaponData.WeaponTag);
        }
    }

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);*/
    
    IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(WeaponOwner.Get());
    if (ASCHolder && ASCHolder->GetAbilitySystemComponent())
    {
        // 태그 제거
        ASCHolder->GetAbilitySystemComponent()->RemoveLooseGameplayTag(CurrentDataAsset->WeaponData.WeaponTag);
    }
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AWeaponBase::ExecuteWeaponEffects(EWeaponActionType ActionType)
{
    if (!CurrentDataAsset) return;
    const FWeaponData& Data = CurrentDataAsset->WeaponData;

    // Fire/Reload 시 사운드 및 VFX 재생
    USoundBase* TargetSound = (ActionType == EWeaponActionType::Fire) ? Data.FireSound.LoadSynchronous() : Data.ReloadSound.LoadSynchronous();
    if (TargetSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, TargetSound, GetMuzzleLocation());
    }

    if (UNiagaraSystem* VFX = Data.MuzzleFlash.LoadSynchronous())
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(VFX, WeaponMesh, Data.MuzzleSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
    }
}

void AWeaponBase::InitializeAttributes()
{
    /*if (!CurrentDataAsset || !WeaponOwner.IsValid()) return;

    UAbilitySystemComponent* ASC = Cast<IAbilitySystemInterface>(WeaponOwner.Get())->GetAbilitySystemComponent();
    if (!ASC || !bIsActiveWeapon) return;

    // 데이터 에셋의 변수를 직접 가져옵니다.
    float MaxAmmoValue = CurrentDataAsset->WeaponData.DefaultMaxAmmo;

    // 태그나 GE를 거치지 않고 직접 AttributeSet의 값을 수정합니다.
    if (UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(ASC->GetSet<UWeaponAttributeSet>()))
    {
        WeaponAS->SetMaxAmmo(MaxAmmoValue);
        WeaponAS->SetCurrentAmmo(MaxAmmoValue); // 초기 탄창 가득 채우기
    }*/
    
    if (!CurrentDataAsset) return;

    // 1. 무기 자체가 태생적으로 가져야 할 탄약 수치를 데이터 에셋에서 가져와 저장합니다.
    CurrentAmmoCount = CurrentDataAsset->WeaponData.DefaultMaxAmmo;

    // 2. 만약 이 무기가 '손에 들린 상태'라면 캐릭터의 실시간 스탯(AttributeSet)도 즉시 업데이트합니다.
    if (bIsActiveWeapon && WeaponOwner.IsValid())
    {
        if (IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(WeaponOwner.Get()))
        {
            UAbilitySystemComponent* ASC = ASCHolder->GetAbilitySystemComponent();
            if (UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(ASC->GetSet<UWeaponAttributeSet>()))
            {
                // 여기서 0/0 현상을 방지하기 위해 직접 값을 밀어넣습니다.
                WeaponAS->SetMaxAmmo(CurrentDataAsset->WeaponData.DefaultMaxAmmo);
                WeaponAS->SetCurrentAmmo(CurrentDataAsset->WeaponData.DefaultMaxAmmo);
            }
        }
    }
}

FVector AWeaponBase::GetMuzzleLocation() const
{
    return (WeaponMesh && CurrentDataAsset) ? WeaponMesh->GetSocketLocation(CurrentDataAsset->WeaponData.MuzzleSocketName) : GetActorLocation();
}