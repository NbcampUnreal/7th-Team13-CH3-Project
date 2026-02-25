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

    // 단발 무기의 경우, 이전 사격의 태그가 아직 남아있다면 중복 실행 방지
    // if (WeaponData->bIsFullAuto) return false;


    /*if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(
        FGameplayTag::RequestGameplayTag(FName("State.Player.IsAttacking"))))
    {
        return false;
    }*/

    return true;
}

void UGA_Attack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Warning, TEXT("ActivateAbility 실행"));
    // Cooldown, Cost가 없다면 종료
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    bIsInputPressed = true;

    UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
    ReleaseTask->OnRelease.AddDynamic(this, &UGA_Attack::OnInputReleased);
    ReleaseTask->ReadyForActivation();

    if (HasAuthority(&CurrentActivationInfo))
    {
        HandleFiringLoop();
    }
}

void UGA_Attack::HandleFiringLoop()
{
    const FWeaponData* WeaponData = GetWeaponData();
    // 발사 실행
    UE_LOG(LogTemp, Warning, TEXT("발사 실행"));
    PlayRecoilMontage();
    SpawnProjectile();
    
    FString RoleString = HasAuthority(&CurrentActivationInfo) ? TEXT("Authority(Server)") : TEXT("Simulated(Client)");
    UE_LOG(LogTemp, Warning, TEXT("[%s] ActivateAbility 실행됨!"), *RoleString);
    
    // 연사 처리
    if (WeaponData->bIsFullAuto)
    {
        // 마우스를 계속 누르고 있는 상태인지 확인
        if (bIsInputPressed)
        {
            UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, WeaponData->DefaultFireRate);
            if (DelayTask)
            {
                // [핵심 2] 연사 루프를 돌 때 OnFinish에 다시 이 함수를 연결
                DelayTask->OnFinish.AddDynamic(this, &UGA_Attack::HandleFiringLoop);
                DelayTask->ReadyForActivation();
            }
        }
        else
        {
            // 마우스를 뗐다면 종료
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        }
    }
    else
    {
        // [핵심 3] 단발 무기는 한 발 쏘고 무조건 종료
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

void UGA_Attack::OnInputReleased(float TimeHeld)
{
    bIsInputPressed = false;
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
