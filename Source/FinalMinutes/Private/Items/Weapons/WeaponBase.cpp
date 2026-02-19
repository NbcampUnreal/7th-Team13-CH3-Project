// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Weapons/WeaponBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "Items/Weapons/WeaponDataAsset.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Subsystems/WeaponRegistrySubsystem.h"


AWeaponBase::AWeaponBase()
{
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();
}

void AWeaponBase::InitializeWeapon(FGameplayTag InWeaponTag, AActor* InOwner)
{
    WeaponOwner = InOwner;
    if (!WeaponOwner.IsValid() || !InWeaponTag.IsValid()) return;

    // 1. RegistrySubsystem을 통해 PrimaryDataAsset 획득
    UGameInstance* GI = GetGameInstance();
    UWeaponRegistrySubsystem* Registry = GI ? GI->GetSubsystem<UWeaponRegistrySubsystem>() : nullptr;
    
    CurrentDataAsset = Registry ? Registry->GetWeaponDataByTag(InWeaponTag) : nullptr;

    if (CurrentDataAsset)
    {
        // 2. TSoftObjectPtr을 이용한 비동기 로딩 요청
        TSoftObjectPtr<USkeletalMesh> MeshPtr = CurrentDataAsset->WeaponData.WeaponMesh;
        
        if (MeshPtr.IsPending())
        {
            FStreamableManager& Streamable = UAssetManager::Get().GetStreamableManager();
            Streamable.RequestAsyncLoad(
                MeshPtr.ToSoftObjectPath(),
                FStreamableDelegate::CreateUObject(this, &AWeaponBase::OnWeaponMeshLoaded, InWeaponTag)
            );
        }
        else
        {
            // 이미 로드되어 있는 경우 즉시 실행
            OnWeaponMeshLoaded(InWeaponTag);
        }
    }
}

void AWeaponBase::OnWeaponMeshLoaded(FGameplayTag InWeaponTag)
{
    // 로딩 완료 시점에 이 객체가 아직 유효한지 확인
    if (!CurrentDataAsset || !WeaponOwner.IsValid()) return;

    // 3. 로드된 메시 적용
    USkeletalMesh* LoadedMesh = CurrentDataAsset->WeaponData.WeaponMesh.Get();
    if (LoadedMesh)
    {
        WeaponMesh->SetSkeletalMesh(LoadedMesh);
        
        // 4. 소켓 부착 로직 실행
        AttachToCharacter();
        
        // 5. GAS Attribute 초기화 로직
        // InitializeWeapon(CurrentDataAsset, WeaponAttributeSet); // 기존 코드 호출
    }
}

void AWeaponBase::AttachToCharacter()
{
    if (!WeaponOwner.IsValid() || !CurrentDataAsset) return;

    // 1. DataAsset에 정의된 소켓 이름 사용 
    FName SocketName = CurrentDataAsset->WeaponData.HandSocketName;
    
    if (USkeletalMeshComponent* ParentMesh = WeaponOwner->FindComponentByClass<USkeletalMeshComponent>())
    {
        // 2. 부착 규칙 설정 (위치/회전/크기를 소켓에 맞춤)
        FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
        AttachToComponent(ParentMesh, AttachRules, SocketName);

        // 3. 데이터 에셋에 정의된 상대적 오프셋 적용
        // SnapToTarget 이후에 상대 좌표를 설정하여 미세 조정합니다.
        WeaponMesh->SetRelativeTransform(CurrentDataAsset->WeaponData.WeaponMeshOffset);
    }
}

void AWeaponBase::InitializeAttributes()
{
    // 1. 유효성 검사 및 ASC 확보
    if (!CurrentDataAsset || !WeaponOwner.IsValid()) return;
    
    IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(WeaponOwner.Get());
    if (!ASCOwner) return;

    UAbilitySystemComponent* ASC = ASCOwner->GetAbilitySystemComponent();
    if (!ASC || !InitStatEffectClass) return;

    // 2. GameplayEffectSpec 생성
    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddInstigator(WeaponOwner.Get(), this);
    
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InitStatEffectClass, 1.0f, EffectContext);
    
    if (SpecHandle.IsValid())
    {
        FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
        const FWeaponData& Data = CurrentDataAsset->WeaponData;

        // 3. SetByCaller를 통해 데이터 테이블 수치 주입
        // 태그는 프로젝트의 TagManager 또는 NativeTag로 정의되어 있어야 합니다.
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.MaxAmmo")), Data.DefaultMaxAmmo);
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.Damage")), Data.DefaultDamage);
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.FireRate")), Data.DefaultFireRate);
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.ReloadSpeed")), Data.DefaultReloadSpeed);
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.BulletSpeed")), Data.DefaultBulletSpeed);
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.EffectiveRange")), Data.DefaultSoundSize); // 예시 매핑

        // 4. 기존 무기 효과가 있다면 제거 후 신규 적용 (무기 교체 대응)
        if (WeaponStatEffectHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(WeaponStatEffectHandle);
        }

        WeaponStatEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);

        // 5. 현재 탄약은 '변화량'이므로 별도의 Instant GE나 직접 설정을 통해 초기화
        // MaxAmmo가 설정된 직후 CurrentAmmo를 MaxAmmo와 동기화 시킵니다.
        if (UWeaponAttributeSet* WeaponAS = const_cast<UWeaponAttributeSet*>(ASC->GetSet<UWeaponAttributeSet>()))
        {
            WeaponAS->SetCurrentAmmo(Data.DefaultMaxAmmo);
        }
    }
}