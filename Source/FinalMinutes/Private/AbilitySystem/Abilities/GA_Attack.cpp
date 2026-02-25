#include "AbilitySystem/Abilities/GA_Attack.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Character/Components/CombatComponent.h"
#include "Items/Projectiles/ProjectileBullet.h"
#include "Items/Weapons/FWeaponData.h"
#include "Items/Weapons/WeaponBase.h"
#include "Items/Weapons/WeaponDataAsset.h"

UGA_Attack::UGA_Attack()
{
    // 액터당 하나의 인스턴스만 생성하여 자원 효율성 높임
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 어빌리티 태그 설정
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Attack")));

    // [핵심] 이 태그는 EndAbility() 호출 시 자동으로 제거됩니다.
    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsAttacking")));

    // 재장전 중에는 실행 불가
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsReloading")));
}

bool UGA_Attack::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    FGameplayTagContainer* OptionalRelevantTags) const
{
    UE_LOG(LogTemp, Warning, TEXT("CanActivateAbility 실행"));
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    const FWeaponData* WeaponData = GetWeaponData();
    if (!WeaponData) return false;

    return true;
}

void UGA_Attack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Warning, TEXT("ActivateAbility 실행"));
    
    // 1. 코스트/쿨다운 체크
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    bIsFirstShot = true;
    
    // 2. 루프 시작
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
    
    if (bIsFirstShot)
    {
        bIsFirstShot = false; // 플래그 변경
    }
    else
    {
        // 두 번째 발사부터: 비용(탄약)만 체크하고 소모합니다. (1발 소모)
        if (!CheckCost(CurrentSpecHandle, CurrentActorInfo))
        {
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
            return;
        }
        ApplyCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
    }

    // 3. 발사 실행
    if (HasAuthority(&CurrentActivationInfo))
    {
        PlayRecoilMontage();
        SpawnProjectile();
    }

    // 4. 연사(FullAuto) 처리
    if (WeaponData->bIsFullAuto)
    {
        // 다음 발사 예약
        // 캐릭터에서 CancelAbilities를 호출하면 이 예약된 태스크도 자동으로 취소되어 루프가 멈춥니다.
        UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, WeaponData->DefaultFireRate);
        DelayTask->OnFinish.AddDynamic(this, &UGA_Attack::HandleFiringLoop);
        DelayTask->ReadyForActivation();
    }
    else
    {
        // 단발 무기면 한 번 쏘고 종료
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UGA_Attack::PlayRecoilMontage()
{
    UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
    if (!MyASC) return;

    UAnimMontage* SelectedMontage = StandRecoilMontage;
    if (MyASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning"))))
        SelectedMontage = ProneRecoilMontage;
    else if (MyASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsCrouching"))))
        SelectedMontage = CrouchRecoilMontage;

    if (SelectedMontage)
    {
        // 상체 Additive 몽타주 1회 재생 (발사 시 툭 치는 느낌)
        UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, SelectedMontage);
        if (MontageTask)
        {
            MontageTask->ReadyForActivation();
        }
    }
}

void UGA_Attack::SpawnProjectile() const
{
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!PlayerChar) return;

    UCombatComponent* CombatComp = PlayerChar->GetCombatComponent();
    AWeaponBase* CurrentWeapon = (CombatComp) ? CombatComp->GetCurrentWeapon() : nullptr;
    if (!CurrentWeapon || !CurrentWeapon->GetCurrentDataAsset()) return;

    const FWeaponData& WeaponData = CurrentWeapon->GetCurrentDataAsset()->WeaponData;
    APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
    if (!PC) return;

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 50000.0f);
    FHitResult HitResult;
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(PlayerChar);

    FVector TargetLocation = TraceEnd;
    if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, TraceParams))
    {
        TargetLocation = HitResult.ImpactPoint;
    }

    FVector MuzzleLocation = CurrentWeapon->GetWeaponMesh()->GetSocketLocation(WeaponData.MuzzleSocketName);
    FRotator AdjustedRotation = (TargetLocation - MuzzleLocation).Rotation();

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    FGameplayEffectContextHandle DamageContext = ASC->MakeEffectContext();
    DamageContext.AddInstigator(PlayerChar, CurrentWeapon);

    FGameplayEffectSpecHandle DamageSpec = ASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, DamageContext);
    if (DamageSpec.IsValid())
    {
        DamageSpec.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.Damage")),
                                                       WeaponData.DefaultDamage);
    }

    CurrentWeapon->ExecuteWeaponEffects(EWeaponActionType::Fire);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = PlayerChar;
    SpawnParams.Instigator = PlayerChar;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AProjectileBullet* Bullet = GetWorld()->SpawnActor<AProjectileBullet>(
        WeaponData.ProjectileClass, MuzzleLocation, AdjustedRotation, SpawnParams);
    if (Bullet)
    {
        Bullet->InitializeProjectile(DamageSpec, WeaponData.DefaultBulletSpeed);
    }
}

void UGA_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                            const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                            bool bWasCancelled)
{
    UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
    if (MyASC && ActiveAttackEffectHandle.IsValid())
    {
        MyASC->RemoveActiveGameplayEffect(ActiveAttackEffectHandle);
        ActiveAttackEffectHandle.Invalidate();
    }

    // 부모 클래스의 EndAbility가 호출되어야 태그 제거가 완료됩니다.
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const FWeaponData* UGA_Attack::GetWeaponData() const
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (Player && Player->GetCombatComponent())
    {
        AWeaponBase* Weapon = Player->GetCombatComponent()->GetCurrentWeapon();
        if (Weapon && Weapon->GetCurrentDataAsset())
        {
            return &Weapon->GetCurrentDataAsset()->WeaponData;
        }
    }
    return nullptr;
}
