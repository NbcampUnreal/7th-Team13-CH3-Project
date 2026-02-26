// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Weapons/WeaponBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
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
    }
}

void AWeaponBase::AttachToCharacter()
{
    if (!WeaponOwner.IsValid() || !CurrentDataAsset) return;

    // 1-1. 캐릭터로부터 ASC 가져오기
    IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(WeaponOwner.Get());
    if (!ASCHolder) return;

    UAbilitySystemComponent* ASC = ASCHolder->GetAbilitySystemComponent();
    if (!ASC) return;

    // 1-2. DataAsset에 설정된 WeaponTag 부여
    // AddLooseGameplayTag는 간단한 상태 표시용으로 적합합니다.
    // 만약 복제가 필요하다면 GameplayEffect를 사용하는 것이 정석입니다.
    ASC->AddLooseGameplayTag(CurrentDataAsset->WeaponData.WeaponTag);

    // 태그가 변경되었음을 알리기 위해 수동으로 업데이트가 필요할 수 있습니다.
    ASC->UpdateTagMap(CurrentDataAsset->WeaponData.WeaponTag, 1);


    // 떨림 방지를 위한 충돌 및 물리 설정 추가 ---
    // 캐릭터와 무기 메시가 겹쳐서 서로 밀어내는 현상을 방지합니다.
    WeaponMesh->SetSimulatePhysics(false); // 물리 시뮬레이션 중지
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌 완전 비활성화
    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore); // 모든 채널 무시

    // 2-1. DataAsset에 정의된 소켓 이름 사용 
    FName SocketName = CurrentDataAsset->WeaponData.HandSocketName;

    USkeletalMeshComponent* ParentMesh = WeaponOwner->FindComponentByClass<USkeletalMeshComponent>();
    if (!ParentMesh) return;

    // 2-2. 부착 규칙 설정 (위치/회전/크기를 소켓에 맞춤)
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepRelative,
        true);
    AttachToComponent(ParentMesh, AttachRules, SocketName);

    // 2-3. 데이터 에셋에 정의된 상대적 오프셋 적용
    // SnapToTarget 이후에 상대 좌표를 설정하여 미세 조정합니다.
    WeaponMesh->SetRelativeTransform(CurrentDataAsset->WeaponData.WeaponMeshOffset);
}

void AWeaponBase::DetachFromCharacter()
{
    if (!WeaponOwner.IsValid() || !CurrentDataAsset) return;

    IAbilitySystemInterface* ASCHolder = Cast<IAbilitySystemInterface>(WeaponOwner.Get());
    if (ASCHolder)
    {
        UAbilitySystemComponent* ASC = ASCHolder->GetAbilitySystemComponent();
        if (ASC)
        {
            // 장착했던 태그 제거
            ASC->RemoveLooseGameplayTag(CurrentDataAsset->WeaponData.WeaponTag);
        }
    }

    // 물리적 분리 로직
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AWeaponBase::ExecuteWeaponEffects(EWeaponActionType ActionType)
{
    if (!CurrentDataAsset) return;
    const FWeaponData& Data = CurrentDataAsset->WeaponData;

    if (ActionType == EWeaponActionType::Fire)
    {
        // 사운드 재생 로직
        if (USoundBase* Sound = Data.FireSound.LoadSynchronous())
        {
            UGameplayStatics::PlaySoundAtLocation(this, Sound, GetMuzzleLocation());
        }

        // VFX(Niagara) 재생 로직
        if (UNiagaraSystem* VFX = Data.MuzzleFlash.LoadSynchronous())
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                VFX,
                WeaponMesh,
                Data.MuzzleSocketName,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::SnapToTarget,
                true
            );
        }
    }

    if (ActionType == EWeaponActionType::Reload)
    {
        // 사운드 재생 로직
        if (USoundBase* Sound = Data.ReloadSound.LoadSynchronous())
        {
            UGameplayStatics::PlaySoundAtLocation(this, Sound, GetMuzzleLocation());
        }

        // VFX(Niagara) 재생 로직
        if (UNiagaraSystem* VFX = Data.MuzzleFlash.LoadSynchronous())
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                VFX,
                WeaponMesh,
                Data.MuzzleSocketName,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::SnapToTarget,
                true
            );
        }
    }
}

FVector AWeaponBase::GetMuzzleLocation() const
{
    if (WeaponMesh && CurrentDataAsset)
    {
        return WeaponMesh->GetSocketLocation(CurrentDataAsset->WeaponData.MuzzleSocketName);
    }
    return GetActorLocation();
}

FVector AWeaponBase::GetCameraTargetLocation() const
{
    APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
    if (!PC) return FVector::ZeroVector;

    // 1. 카메라의 위치와 회전값 가져오기
    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // 2. 카메라 앞방향으로 아주 먼 거리까지 트레이스 (예: 50,000 unit)
    FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 50000.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner()); // 사격하는 본인 무시

    // 3. 라인 트레이스 실행
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, Params);

    // 무언가 맞았다면 그 지점을, 아니면 트레이스 끝 지점을 반환
    return bHit ? HitResult.ImpactPoint : TraceEnd;
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
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.MaxAmmo")),
                                      Data.DefaultMaxAmmo);
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.Damage")), Data.DefaultDamage);
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.FireRate")),
                                      Data.DefaultFireRate);
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.ReloadSpeed")),
                                      Data.DefaultReloadSpeed);
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.BulletSpeed")),
                                      Data.DefaultBulletSpeed);
        Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.EffectiveRange")),
                                      Data.DefaultSoundSize); // 예시 매핑

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
