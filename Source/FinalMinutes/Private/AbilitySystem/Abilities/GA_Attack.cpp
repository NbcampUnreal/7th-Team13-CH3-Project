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

UGA_Attack::UGA_Attack()
{
    // 어빌리티 인스턴싱 정책: 액터당 하나 생성
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 어빌리티 및 상태 태그 설정
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Attack")));
    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsAttacking")));

    // 실행 차단 태그: 장전 중에는 공격 불가
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsReloading")));
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

void UGA_Attack::GenerateFiringNoise() const
{
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!PlayerCharacter) return;

    UCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent();
    AWeaponBase* CurrentWeapon = CombatComponent ? CombatComponent->GetActiveWeapon() : nullptr;
    if (!CurrentWeapon || !CurrentWeapon->GetCurrentDataAsset()) return;
    
    float SoundRange = CurrentWeapon->GetFinalSoundSize();
    
    CurrentWeapon->MakeNoise(SoundRange, PlayerCharacter, CurrentWeapon->GetActorLocation());

    #if !UE_BUILD_SHIPPING
    DrawDebugSphere(
        GetWorld(),
        CurrentWeapon->GetActorLocation(),
        SoundRange,
        32,
        FColor::Orange,
        false,
        1.0f);
    #endif
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
    FRotator ShootRotation = (TargetLocation - MuzzleLocation).Rotation();

    // 3. 데미지 정보 생성
    FGameplayEffectSpecHandle DamageEffectSpecHandle = CreateDamageSpec(PlayerCharacter, CurrentWeapon, WeaponData.DefaultDamage);
    
    // 4. 발사 연출 및 스폰
    CurrentWeapon->ExecuteWeaponEffects(EWeaponActionType::Fire);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = PlayerCharacter;
    SpawnParams.Instigator = PlayerCharacter;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (AProjectileBullet* Bullet = GetWorld()->SpawnActor<AProjectileBullet>(
        WeaponData.ProjectileClass, MuzzleLocation, ShootRotation, SpawnParams))
    {
        Bullet->InitializeProjectile(DamageEffectSpecHandle, WeaponData.DefaultBulletSpeed);
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
