#include "AbilitySystem/Attributes/CharacterAttributeSet.h"

#include "AIController.h"
#include "GameplayEffectExtension.h"
#include "Framework/FinalMinutesGameMode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Framework/FinalMinutesGameState.h"
#include "Monster/AMonsterCharacter.h"

UCharacterAttributeSet::UCharacterAttributeSet()
{
    // 초기화
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
	   
    InitStamina(100.0f);
    InitMaxStamina(100.0f);
    
    InitAttackDamage(10.0f);
    InitDefence(0.0f);
    InitMoveSpeed(300.0f);
}

// Attribute값이 변하기 전에 실행
void UCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);
	
    if (Attribute == GetMaxHealthAttribute())
    {
        // 최대체력 보정 (최대 체력은 1보다 작을 수 없음)
        NewValue = FMath::Max(NewValue, 1.0f);
    }
    else if (Attribute == GetMoveSpeedAttribute())
    {
        // 값 보정만 담당
        NewValue = FMath::Max(NewValue, 0.0f);
    }
}

// Attribute값이 바뀌고 나서 실행
void UCharacterAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    if (Attribute == GetMoveSpeedAttribute())
    {
        AActor* TargetActor = GetOwningAbilitySystemComponent()->GetAvatarActor();
        if (!TargetActor) return;
        ACharacter* Character = Cast<ACharacter>(TargetActor);
        if (!Character) return;
        Character->GetCharacterMovement()->MaxWalkSpeed = NewValue;
    }
}


// GameplayEffect 실행 뒤에 실행
void UCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
	
    // 현재 변경된 Attribute가 무엇인지 확인한다. Health면 Health에 맞게 보정처리
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // GetHealth값이 최소 0.0f, 최대 GetMaxHealth를 넘지 않게 보정해준다.
        // ex) 최대체력 100, 현재 체력90일때 체력 20을 회복시켜주는 힐팩을 먹음 -> 현재체력 110 -> 최대체력 넘음 (x) 잘못됨
        // 그래서 보정을 해줌 0.0f에서 최대체력 100사이
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
    }
    else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
    }
    else if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        // Health에서 바로 깎지 않는 이유는 도트데미지일수도 있어서
        
        // 들어온 기초 데미지 저장 및 초기화
        const float DamageValue = GetDamage();
        SetDamage(0.f);
        
        // 데미지가 0이하면 무시
        if (DamageValue <= 0.f) return;
        
        // 최종 데미지 계산용 변수
        float FinalDamage = DamageValue;
        
        // 피격자(몬스터)로부터 배율 정보 가져오기
        if (AAMonsterCharacter* TargetMonster = Cast<AAMonsterCharacter>(Data.Target.GetAvatarActor()))
        {
            // 총알이 담아온 태그에서 정보 추출
            FGameplayTagContainer SpecAssetTags;
            Data.EffectSpec.GetAllAssetTags(SpecAssetTags);
            
            // 데미지 계산
            const float Multiplier = TargetMonster->GetDamageMultiplierForRegion(SpecAssetTags);
            FinalDamage = DamageValue * Multiplier;
            
            UE_LOG(LogTemp, Log, TEXT("부위 판정: %f * %f = 최종 %f"), DamageValue, Multiplier, FinalDamage);
        }
        
        // 체력 차감
        const float NewHealth = FMath::Clamp(GetHealth() - FinalDamage, 0.0f, GetMaxHealth());
        SetHealth(NewHealth);
        
        // 데미지를 받고서 피가 0이상이면 피격효과 
        HandleHitReaction(FinalDamage);

        // 피가 0 이하면 사망로직
        if (NewHealth <= 0)
        {
            HandleDeath();
        }
    }
}

void UCharacterAttributeSet::HandleDeath() const
{
    AActor* AvatarActor = GetOwningActor();
    UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
    
    if (!ASC) return;
    
    FGameplayEventData Payload;
    Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Montage.Death"));
    
    // 플레이어인 경우 : 기존의 자세별 사망 모션 실행
    if (AvatarActor->ActorHasTag(FName("Player")))
    {
        if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsCrouching"))))
        {
            Payload.EventMagnitude = 1.0f;
        }
        else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Player.IsProning"))))
        {
            Payload.EventMagnitude = 2.0f;
        }
        else
        {
            Payload.EventMagnitude = 0.0f;
        } 
    }
    else
    {
        //킬카운트 증가
        AFinalMinutesGameState* GS = GetWorld()->GetGameState<AFinalMinutesGameState>();
        if (GS) GS->AddKill();
        
        APawn* MonsterPawn = Cast<APawn>(AvatarActor);
        if (MonsterPawn)
        {
            if (AAIController* AIC = Cast<AAIController>(MonsterPawn->GetController()))
            {
                if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
                {
                    // 블랙보드 키 이름은 에디터와 동일하게 "bisDead"로 설정
                    BB->SetValueAsBool(TEXT("bIsDead"), true);
                    
                    // 물리 관성을 즉시 제거하여 제자리에 얼립니다.
                    if (UPawnMovementComponent* MoveComp = MonsterPawn->GetMovementComponent())
                    {
                        MoveComp->StopMovementImmediately();
                    }
                }
            }
        }
        return;
    }
    ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
    
    // 사망 로직이 발동 된 이후 GameOver
    if (AvatarActor)
    {
        if (AvatarActor->ActorHasTag(FName("Player")))
        {
            AFinalMinutesGameMode* GM = Cast<AFinalMinutesGameMode>(AvatarActor->GetWorld()->GetAuthGameMode());
            if (GM) GM->GameOver();
        }
        else
        {
            // 시체가 10초 뒤 사라지도록 설정
            AvatarActor->SetLifeSpan(10.0f);
        }
    }
}

void UCharacterAttributeSet::HandleHitReaction(const float DamageValue) const
{
    // GameplayCue에 넘길 파라미터
    FGameplayCueParameters Parameters;
    Parameters.RawMagnitude = DamageValue;
    
    // 태그를 가진 Cue를 실행
    const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Character.Damaged"));
    UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
    if (!ASC) return;
    ASC->ExecuteGameplayCue(DamageTag, Parameters);
}

