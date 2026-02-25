#include "Items/Weapons/WeaponBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Items/Weapons/WeaponDataAsset.h"
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
    if (!CurrentDataAsset) return;

    // [수정] .Get() 대신 .LoadSynchronous()를 사용해야 합니다.
    // Soft Pointer는 이름 그대로 '부드러운' 참조일 뿐이므로, 실제 데이터가 필요할 땐 로드해야 합니다.
    USkeletalMesh* LoadedMesh = CurrentDataAsset->WeaponData.WeaponMesh.LoadSynchronous();

    if (LoadedMesh)
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
        
        UE_LOG(LogTemp, Log, TEXT("WeaponBase: Mesh Loaded and Attached for %s"), *InWeaponTag.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponBase: Failed to Load SkeletalMesh for %s"), *InWeaponTag.ToString());
    }
}

void AWeaponBase::AttachToCharacter()
{
    IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(WeaponOwner.Get());
    if (ASCHolder && ASCHolder->GetAbilitySystemComponent())
    {
        // 장착 태그 부여 (중첩 방지를 위해 하나만 부여)
        ASCHolder->GetAbilitySystemComponent()->AddLooseGameplayTag(CurrentDataAsset->WeaponData.WeaponTag);
    }

    USkeletalMeshComponent* ParentMesh = WeaponOwner->FindComponentByClass<USkeletalMeshComponent>();
    if (ParentMesh)
    {
        AttachToComponent(ParentMesh, FAttachmentTransformRules::SnapToTargetIncludingScale,
                          CurrentDataAsset->WeaponData.HandSocketName);
        WeaponMesh->SetRelativeTransform(CurrentDataAsset->WeaponData.WeaponMeshOffset);
    }
}

void AWeaponBase::DetachFromCharacter()
{
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
    USoundBase* TargetSound = (ActionType == EWeaponActionType::Fire)
                                  ? Data.FireSound.LoadSynchronous()
                                  : Data.ReloadSound.LoadSynchronous();
    if (TargetSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, TargetSound, GetMuzzleLocation());
    }

    if (UNiagaraSystem* VFX = Data.MuzzleFlash.LoadSynchronous())
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(VFX, WeaponMesh, Data.MuzzleSocketName, FVector::ZeroVector,
                                                     FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
    }
}

void AWeaponBase::InitializeAttributes()
{
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
    return (WeaponMesh && CurrentDataAsset)
               ? WeaponMesh->GetSocketLocation(CurrentDataAsset->WeaponData.MuzzleSocketName)
               : GetActorLocation();
}
