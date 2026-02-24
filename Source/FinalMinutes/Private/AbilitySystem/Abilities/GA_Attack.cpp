#include "AbilitySystem/Abilities/GA_Attack.h"
#include "Character/Player/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Components/CombatComponent.h"
#include "Items/Projectiles/ProjectileBullet.h"
#include "Items/Weapons/FWeaponData.h"
#include "Items/Weapons/WeaponBase.h"
#include "Items/Weapons/WeaponDataAsset.h"

UGA_Attack::UGA_Attack()
{
    // 어빌리티가 실행될때 액터당 하나의 인스턴스(객체)만 생성해서 재사용하겠다.
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // Ability Tags, 어빌리티 자체에 붙여주는 태그
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.Attack")));

    // 소유태그 / 실행중 어떤 태그를 가질지
    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsAttacking")));
}

bool UGA_Attack::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    // const 안정성을 위해서 매개변수의 ActorInfo를 직접 쓰는게 좋다고 한다.
    APlayerCharacter* Player = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!Player) return false;

    // 이 부분은 나중에 무기정보같은곳에서 연사인지 단발인지 판단을 해서 분기하면 될듯
    if (!bIsRepeat)
    {
        FGameplayTag AttackingTag = FGameplayTag::RequestGameplayTag(FName("State.Player.IsAttacking"));
        if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(AttackingTag))
        {
            return false;
        }
    }

    return true;
}

void UGA_Attack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // 스킬 사용을 위한 조건 체크 , 쿨타임, 코스트다 있는지
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        // Commit 실패 (스태미너 부족, Cooldown 중 등)
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    bIsInputPressed = true;

    UAbilityTask_WaitInputRelease* ReleaseTask;
    // 연사모드면 키 떼기 감지
    if (bIsRepeat)
    {
        ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
        ReleaseTask->OnRelease.AddDynamic(this, &UGA_Attack::OnInputReleased);
        ReleaseTask->ReadyForActivation();
    }

    PlayAttack();
}

// 재사용 하기 위해 공격 로직 분리
void UGA_Attack::PlayAttack()
{
    UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
    UAnimMontage* SelectedMontage = StandAttackMontage;
    APlayerCharacter* Player = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (MyASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning"))))
    {
        SelectedMontage = ProneAttackMontage;
    }
    else if (MyASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsCrouching"))))
    {
        SelectedMontage = CrouchAttackMontage;
    }

    if (SelectedMontage)
    {
        UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            SelectedMontage
        );
        if (MontageTask)
        {
            MontageTask->OnCompleted.AddDynamic(this, &UGA_Attack::OnMontageEnded);
            MontageTask->OnInterrupted.AddDynamic(this, &UGA_Attack::OnMontageEnded);
            MontageTask->OnCancelled.AddDynamic(this, &UGA_Attack::OnMontageEnded);

            MontageTask->ReadyForActivation();
        }
    }
    // Gameplay Event 대기
    UAbilityTask_WaitGameplayEvent* WaitEventTask =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this,
            FGameplayTag::RequestGameplayTag(FName("Event.Montage.Attack"))
        );

    if (WaitEventTask)
    {
        WaitEventTask->EventReceived.AddDynamic(this, &UGA_Attack::OnAttackGameplayEvent);
        WaitEventTask->ReadyForActivation();
        SpawnProjectile();
    }
}

void UGA_Attack::OnInputReleased(float TimeHeld)
{
    // EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    bIsInputPressed = false;
}

void UGA_Attack::OnAttackGameplayEvent(FGameplayEventData EventData)
{
    APlayerCharacter* Character = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    if (!Character) return;
    UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
    if (!MyASC) return;


    // 컨텍스트 생성, 효과가 어디서부터 나타났는지, 추후에 데미지 계산이나 로그 시스템에서 누가 사용한지 알 수 있음
    FGameplayEffectContextHandle EffectContext = MyASC->MakeEffectContext();
    EffectContext.AddSourceObject(Character);

    // 실제로 적용될 효과를 작성, GE클래스
    FGameplayEffectSpecHandle SpecHandle;

    SpecHandle = MyASC->MakeOutgoingSpec(
        AttackEffectClass,
        1.0f, // Level
        EffectContext
    );


    if (SpecHandle.IsValid())
    {
        ActiveAttackEffectHandle = MyASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void UGA_Attack::SpawnProjectile()
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Avatar);
    if (!PlayerChar) return;

    UCombatComponent* CombatComp = PlayerChar->GetCombatComponent();
    if (!CombatComp) return;

    AWeaponBase* CurrentWeapon = CombatComp->GetCurrentWeapon();
    if (!CurrentWeapon || !CurrentWeapon->GetCurrentDataAsset()) return;

    const FWeaponData& WeaponData = CurrentWeapon->GetCurrentDataAsset()->WeaponData;

    // --- 1. 카메라 기반 목표 지점(Target Point) 계산 ---
    APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
    if (!PC) return;

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // 카메라 정면으로 먼 거리까지 라인 트레이스
    FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 50000.0f);
    FHitResult HitResult;
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(Avatar); // 시전자 무시

    FVector TargetLocation = TraceEnd;
    if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, TraceParams))
    {
        TargetLocation = HitResult.ImpactPoint;
    }

    // --- 2. 총구 위치 및 보정된 발사 회전값 계산 ---
    // 실제 총알이 나갈 물리적 위치 (데이터 에셋의 소켓 이름 사용)
    FVector MuzzleLocation = CurrentWeapon->GetWeaponMesh()->GetSocketLocation(WeaponData.MuzzleSocketName);

    // 총구에서 목표 지점을 바라보는 방향 벡터 생성
    FVector LaunchDir = (TargetLocation - MuzzleLocation).GetSafeNormal();
    FRotator AdjustedRotation = LaunchDir.Rotation();

    // --- 3. GAS 데미지 스펙 생성 (기존 로직 유지) ---
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddInstigator(Avatar, CurrentWeapon);

    FGameplayEffectSpecHandle DamageSpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, EffectContext);
    if (DamageSpecHandle.IsValid())
    {
        // 데이터 에셋의 기본 데미지 수치 주입
        DamageSpecHandle.Data.Get()->SetSetByCallerMagnitude(
            FGameplayTag::RequestGameplayTag(FName("Data.Stat.Damage")), WeaponData.DefaultDamage);
    }

    // --- 4. 투사체 스폰 (보정된 AdjustedRotation 사용) ---
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Avatar;
    SpawnParams.Instigator = Cast<APawn>(Avatar);

    // MuzzleRotation 대신 보정된 AdjustedRotation을 넣습니다.
    AProjectileBullet* Bullet = GetWorld()->SpawnActor<AProjectileBullet>(
        WeaponData.ProjectileClass, MuzzleLocation, AdjustedRotation, SpawnParams);

    if (Bullet)
    {
        // 데이터 에셋의 탄속 수치 주입
        Bullet->InitializeProjectile(DamageSpecHandle, WeaponData.DefaultBulletSpeed);
    }
}

void UGA_Attack::OnMontageEnded()
{
    // 몽타주가 끝났을때 판단
    if (bIsRepeat && bIsInputPressed)
    {
        // 아직 좌클릭을 누르고 있으면 다시 공격
        // 쿨타임/코스트 체크를 위해 Commit 다시 수행
        if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
        {
            PlayAttack();
        }
        else
        {
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        }
    }
    else
    {
        // 손을 뗐거나 단발이면 종료
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UGA_Attack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
    if (!MyASC) return;
    if (ActiveAttackEffectHandle.IsValid())
    {
        MyASC->RemoveActiveGameplayEffect(ActiveAttackEffectHandle);
        ActiveAttackEffectHandle.Invalidate();
    }

    // 부모 클래스 호출 (Tag 제거 등)
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
