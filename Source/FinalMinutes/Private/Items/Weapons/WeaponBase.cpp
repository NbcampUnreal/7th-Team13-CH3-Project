#include "Items/Weapons/WeaponBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Items/Weapons/WeaponDataAsset.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/WeaponRegistrySubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

AWeaponBase::AWeaponBase()
{
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    // 무기 기본 충돌 설정
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetSimulatePhysics(false);
}

void AWeaponBase::InitializeWeapon(FGameplayTag InWeaponTag, AActor* InOwner)
{
    WeaponOwner = InOwner;

    // 레지스트리 서브시스템에서 데이터 에셋 획득
    UWeaponRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UWeaponRegistrySubsystem>();
    if (!Registry) return;

    CurrentDataAsset = Registry->GetWeaponDataByTag(InWeaponTag);
    if (!CurrentDataAsset) return;

    StartAsyncMeshLoad(InWeaponTag);
}

void AWeaponBase::StartAsyncMeshLoad(FGameplayTag InWeaponTag)
{
    FSoftObjectPath MeshPath = CurrentDataAsset->WeaponData.WeaponMesh.ToSoftObjectPath();
    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

    // 진행 중인 로딩 취소 후 새로 시작
    if (WeaponMeshLoadHandle.IsValid() && WeaponMeshLoadHandle->IsLoadingInProgress())
    {
        WeaponMeshLoadHandle->CancelHandle();
    }

    WeaponMeshLoadHandle = Streamable.RequestAsyncLoad(
        MeshPath,
        FStreamableDelegate::CreateUObject(this, &AWeaponBase::OnWeaponMeshLoaded, InWeaponTag)
    );
}

void AWeaponBase::OnWeaponMeshLoaded(FGameplayTag InWeaponTag)
{
    if (!CurrentDataAsset || !WeaponMeshLoadHandle.IsValid()) return;

    USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(WeaponMeshLoadHandle->GetLoadedAsset());
    if (!LoadedMesh)
    {
        WeaponMeshLoadHandle.Reset();
        return;
    }

    WeaponMesh->SetSkeletalMesh(LoadedMesh);

    if (!bIsActiveWeapon)
    {
        SetActorHiddenInGame(true);
        WeaponMeshLoadHandle.Reset();
        return;
    }

    SetActorHiddenInGame(false);
    AttachToCharacter();
    WeaponMeshLoadHandle.Reset();
}

void AWeaponBase::AttachToCharacter()
{
    UpdateOwnerGameplayTag(true);

    if (!WeaponOwner.IsValid() || !CurrentDataAsset) return;

    USkeletalMeshComponent* ParentMesh = WeaponOwner->FindComponentByClass<USkeletalMeshComponent>();
    if (!ParentMesh) return;

    AttachToComponent(
        ParentMesh,
        FAttachmentTransformRules::SnapToTargetIncludingScale,
        CurrentDataAsset->WeaponData.HandSocketName);

    WeaponMesh->SetRelativeTransform(CurrentDataAsset->WeaponData.WeaponMeshOffset);
}

void AWeaponBase::DetachFromCharacter()
{
    UpdateOwnerGameplayTag(false);
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AWeaponBase::UpdateOwnerGameplayTag(bool bAddTag)
{
    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC || !CurrentDataAsset) return;

    if (bAddTag)
    {
        ASC->AddLooseGameplayTag(CurrentDataAsset->WeaponData.WeaponTag);
    }
    else
    {
        ASC->RemoveLooseGameplayTag(CurrentDataAsset->WeaponData.WeaponTag);
    }
}

void AWeaponBase::ExecuteWeaponEffects(EWeaponActionType ActionType)
{
    if (!CurrentDataAsset) return;

    PlayActionSound(ActionType);
    SpawnActionEffects();
}

void AWeaponBase::PlayActionSound(EWeaponActionType ActionType)
{
    const FWeaponData& Data = CurrentDataAsset->WeaponData;
    USoundBase* TargetSound = (ActionType == EWeaponActionType::Fire)
                                  ? Data.FireSound.LoadSynchronous()
                                  : Data.ReloadSound.LoadSynchronous();

    if (!TargetSound) return;
    UGameplayStatics::PlaySoundAtLocation(this, TargetSound, GetMuzzleLocation());
}

void AWeaponBase::SpawnActionEffects()
{
    const FWeaponData& Data = CurrentDataAsset->WeaponData;
    UNiagaraSystem* VFX = Data.MuzzleFlash.LoadSynchronous();
    if (!VFX) return;

    UNiagaraFunctionLibrary::SpawnSystemAttached(
        VFX,
        WeaponMesh,
        Data.MuzzleSocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget,
        true);
}

void AWeaponBase::InitializeAttributes()
{
    if (!CurrentDataAsset) return;

    // 1. 탄약 수치 초기화 (공통)
    CurrentAmmoCount = CurrentDataAsset->WeaponData.DefaultMaxAmmo;

    // 2. 활성화 상태가 아니거나 주인이 없으면 중단
    if (!bIsActiveWeapon) return;

    UAbilitySystemComponent* ASC = GetOwnerASC();
    if (!ASC) return;

    // 3. 어트리뷰트 셋 확인 및 값 설정
    UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(ASC->GetSet<UWeaponAttributeSet>());
    if (!WeaponAS) return;

    WeaponAS->SetMaxAmmo(CurrentDataAsset->WeaponData.DefaultMaxAmmo);
    WeaponAS->SetCurrentAmmo(CurrentDataAsset->WeaponData.DefaultMaxAmmo);
}

UAbilitySystemComponent* AWeaponBase::GetOwnerASC() const
{
    if (!WeaponOwner.IsValid()) return nullptr;

    IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(WeaponOwner.Get());
    return ASCHolder ? ASCHolder->GetAbilitySystemComponent() : nullptr;
}

FVector AWeaponBase::GetMuzzleLocation() const
{
    if (!WeaponMesh || !CurrentDataAsset)
    {
        return GetActorLocation();
    }

    return WeaponMesh->GetSocketLocation(CurrentDataAsset->WeaponData.MuzzleSocketName);
}
