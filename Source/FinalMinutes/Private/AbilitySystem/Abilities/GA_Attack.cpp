#include "AbilitySystem/Abilities/GA_Attack.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Character/Components/CombatComponent.h"
#include "Items/Projectiles/ProjectileBullet.h"
#include "Items/Weapons/FWeaponData.h"
#include "Items/Weapons/WeaponBase.h"
#include "Items/Weapons/WeaponDataAsset.h"
#include "Perception/AISense_Hearing.h"

UGA_Attack::UGA_Attack()
{
    // 어빌리티 인스턴싱 정책: 액터당 하나 생성
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    
}

bool UGA_Attack::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    FGameplayTagContainer* OptionalRelevantTags) const
{
    return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && GetWeaponData()
        != nullptr;
}

void UGA_Attack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // 첫 발 소모 및 쿨다운 적용
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    bIsFirstShot = true;
    HandleFiringLoop();
}

void UGA_Attack::HandleFiringLoop()
{
    const FWeaponData* WeaponData = GetWeaponData();
    if (!WeaponData)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    // 1. 탄약 체크 및 소모
    if (!bIsFirstShot)
    {
        if (!CheckCost(CurrentSpecHandle, CurrentActorInfo))
        {
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
            return;
        }
        ApplyCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
    }
    bIsFirstShot = false;

    // 2. 발사 실행
    if (HasAuthority(&CurrentActivationInfo))
    {
        ApplyRecoil();
        PlayRecoilMontage();
        SpawnProjectile();
        GenerateFiringNoise();
    }

    // 3. 연사/단발 결정 로직 간소화
    if (!WeaponData->bIsFullAuto)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    // 연사 예약
    UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, WeaponData->DefaultFireRate);
    DelayTask->OnFinish.AddDynamic(this, &UGA_Attack::HandleFiringLoop);
    DelayTask->ReadyForActivation();
}

void UGA_Attack::PlayRecoilMontage()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC) return;

    // 자세별 몽타주 선택
    UAnimMontage* SelectedMontage = StandRecoilMontage;

    if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning"))))
    {
        SelectedMontage = ProneRecoilMontage;
    }
    else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsCrouching"))))
    {
        SelectedMontage = CrouchRecoilMontage;
    }

    if (SelectedMontage)
    {
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SelectedMontage)->
            ReadyForActivation();
    }
}

void UGA_Attack::ApplyRecoil()
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!Player) return;
    UCombatComponent* CombatComponent = Player->GetCombatComponent();
    if (!CombatComponent) return;
    CombatComponent->ApplyRecoil();
}

void UGA_Attack::GenerateFiringNoise() const
{
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!PlayerCharacter) return;

    UCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent();
    AWeaponBase* CurrentWeapon = CombatComponent ? CombatComponent->GetActiveWeapon() : nullptr;
    if (!CurrentWeapon || !CurrentWeapon->GetCurrentDataAsset()) return;
    

    UAISense_Hearing::ReportNoiseEvent(
            GetWorld(),
            PlayerCharacter->GetActorLocation(),
            1.0f,
            PlayerCharacter,
            3000.0f
         ); 
}

void UGA_Attack::SpawnProjectile() const
{
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!PlayerCharacter) return;

    UCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent();
    AWeaponBase* CurrentWeapon = CombatComponent ? CombatComponent->GetActiveWeapon() : nullptr;
    if (!CurrentWeapon || !CurrentWeapon->GetCurrentDataAsset()) return;

    const FWeaponData& WeaponData = CurrentWeapon->GetCurrentDataAsset()->WeaponData;
    APlayerController* PlayerController = Cast<APlayerController>(PlayerCharacter->GetController());
    if (!PlayerController) return;

    // 1. 타겟 지점 계산
    FVector TargetLocation = GetTargetLocation(PlayerController);

    // 2. 사격 방향 설정
    FVector MuzzleLocation = CurrentWeapon->GetWeaponMesh()->GetSocketLocation(WeaponData.MuzzleSocketName);
    const FVector BaseShootDir = (TargetLocation - MuzzleLocation).GetSafeNormal();

    // 3) 샷건/단일탄 설정
    const int32 PelletCount = FMath::Max(1, WeaponData.PelletCount);
    const bool bUseSpread = WeaponData.bUsePelletSpread && (PelletCount > 1);
    const float SpreadRad = FMath::DegreesToRadians(FMath::Max(0.0f, WeaponData.PelletSpreadHalfAngleDeg));

    // 4) 발사 연출은 방아쇠 1회 기준 1번만
    CurrentWeapon->ExecuteWeaponEffects(EWeaponActionType::Fire);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = PlayerCharacter;
    SpawnParams.Instigator = PlayerCharacter;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 같은 발사(한 번 방아쇠)에서 나온 총알들끼리 서로 충돌 무시 처리
    TArray<AProjectileBullet*> SpawnedPellets;
    SpawnedPellets.Reserve(PelletCount);

    // 5) 총알(또는 단일탄) 스폰
    for (int32 i = 0; i < PelletCount; ++i)
    {
        FVector ShootDir = BaseShootDir;

        if (bUseSpread && SpreadRad > 0.0f)
        {
            ShootDir = FMath::VRandCone(BaseShootDir, SpreadRad);
        }

        // 스폰 회전이 곧 ProjectileBullet의 GetActorForwardVector()가 됩니다.
        const FRotator ShootRotation = ShootDir.Rotation();

        const float PelletDamage = WeaponData.DefaultDamage * WeaponData.PelletDamageMultiplier;
        const FGameplayEffectSpecHandle DamageEffectSpecHandle =
            CreateDamageSpec(PlayerCharacter, CurrentWeapon, PelletDamage);

        if (AProjectileBullet* Bullet = GetWorld()->SpawnActor<AProjectileBullet>(
            WeaponData.ProjectileClass, MuzzleLocation, ShootRotation, SpawnParams))
        {
            // 유효 사거리 세팅(InitializeProjectile 시그니처 변경 없음)
            Bullet->SetEffectiveRange(WeaponData.EffectiveRange);

            // 1) 기존 InitializeProjectile만 사용
            Bullet->InitializeProjectile(DamageEffectSpecHandle, WeaponData.DefaultBulletSpeed);

            // 2) 이미 스폰된 총알들과 상호 Ignore 등록 (총알끼리 충돌 방지)
            for (AProjectileBullet* Prev : SpawnedPellets)
            {
                if (!Prev) continue;

                Bullet->IgnoreOtherProjectile(Prev);
                Prev->IgnoreOtherProjectile(Bullet);
            }

            SpawnedPellets.Add(Bullet);
        }
    }
}

FVector UGA_Attack::GetTargetLocation(APlayerController* PC) const
{
    FVector ViewLocation;
    FRotator ViewRotation;
    PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

    FVector TraceEnd = ViewLocation + (ViewRotation.Vector() * 50000.0f);
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetAvatarActorFromActorInfo());

    return GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, ECC_Visibility, Params)
               ? HitResult.ImpactPoint
               : TraceEnd;
}

FGameplayEffectSpecHandle UGA_Attack::CreateDamageSpec(APlayerCharacter* Instigator, AWeaponBase* Weapon,
                                                       float Damage) const
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    Context.AddInstigator(Instigator, Weapon);

    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
    if (SpecHandle.IsValid())
    {
        FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Effect.Damage"));
        SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, Damage);
    }
    
    return SpecHandle;
}

void UGA_Attack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (ActiveAttackEffectHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(ActiveAttackEffectHandle);
            ActiveAttackEffectHandle.Invalidate();
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const FWeaponData* UGA_Attack::GetWeaponData() const
{
    const APlayerCharacter* Player = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!Player || !Player->GetCombatComponent()) return nullptr;

    const AWeaponBase* Weapon = Player->GetCombatComponent()->GetActiveWeapon();
    return (Weapon && Weapon->GetCurrentDataAsset()) ? &Weapon->GetCurrentDataAsset()->WeaponData : nullptr;
}
