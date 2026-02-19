// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_Fire.h"

#include "AbilitySystem/Attributes/WeaponAttributeSet.h"
#include "Character/Components/CombatComponent.h"
#include "Character/Player/PlayerCharacter.h"
#include "Items/Projectiles/ProjectileBullet.h"
#include "Items/Weapons/FWeaponData.h"
#include "Items/Weapons/WeaponDataAsset.h"
#include "Items/Weapons/WeaponBase.h"

UGA_Fire::UGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Fire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
    
    
	// 1. 탄약 체크 (WeaponAttributeSet 참조)
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    const UWeaponAttributeSet* WeaponAS = ASC ? ASC->GetSet<UWeaponAttributeSet>() : nullptr;
    /*
    if (!WeaponAS || WeaponAS->GetCurrentAmmo() <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("탄약체크 실행 여부"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 2. 어빌리티 커밋 (Cost/Cooldown 적용)
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("어빌리티 커밋 실행 여부"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
*/
    UE_LOG(LogTemp, Warning, TEXT("투사체 생성 로직 실행 여부"));
    // 3. 투사체 생성 로직 실행
    SpawnProjectile();

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Fire::SpawnProjectile()
{
    UE_LOG(LogTemp, Warning, TEXT("SpawnProjectile 실행 여부"));
	AActor* Avatar = GetAvatarActorFromActorInfo();
    
    // 1. 캐릭터로 캐스팅
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Avatar);
    if (!PlayerChar) return;

    // 2. CombatComponent를 통해 현재 무기 획득
    // (PlayerCharacter에 GetCombatComponent() 게터가 있다고 가정합니다)
    UCombatComponent* CombatComp = PlayerChar->GetCombatComponent();
    if (!CombatComp) return;
    // 현재 캐릭터가 들고 있는 무기 액터 획득
    AWeaponBase* CurrentWeapon = CombatComp->GetCurrentWeapon(); // 실제 구현 시 캐릭터에서 가져오는 로직 필요
    
    if (!CurrentWeapon || !CurrentWeapon->GetCurrentDataAsset()) return;
    UE_LOG(LogTemp, Warning, TEXT("CurrentWeapon 현재 무기 있음 실행 여부"));
    
    const FWeaponData& WeaponData = CurrentWeapon->GetCurrentDataAsset()->WeaponData;

    // 4. 발사 위치 및 방향 계산 (Muzzle Socket)
    FVector MuzzleLocation = CurrentWeapon->GetWeaponMesh()->GetSocketLocation(WeaponData.MuzzleSocketName);
    FRotator MuzzleRotation = CurrentWeapon->GetWeaponMesh()->GetSocketRotation(WeaponData.MuzzleSocketName);

    // 5. 투사체에 전달할 데미지 Spec 생성
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddInstigator(Avatar, CurrentWeapon);

    FGameplayEffectSpecHandle DamageSpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, EffectContext);
    if (DamageSpecHandle.IsValid())
    {
        // 데이터 에셋의 데미지 수치를 SetByCaller로 주입
        DamageSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Stat.Damage")), WeaponData.DefaultDamage);
    }

    // 6. 투사체 스폰 및 초기화
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Avatar;
    SpawnParams.Instigator = Cast<APawn>(Avatar);

    AProjectileBullet* Bullet = GetWorld()->SpawnActor<AProjectileBullet>(WeaponData.ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);
    if (Bullet)
    {
        UE_LOG(LogTemp, Warning, TEXT("Bullet 현재 총알 있음"));
        Bullet->InitializeProjectile(DamageSpecHandle, WeaponData.DefaultBulletSpeed);
    }
}
