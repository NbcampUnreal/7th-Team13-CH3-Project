// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

UINTERFACE()
class UDamageable : public UInterface
{
    GENERATED_BODY()
};

/**
 * IDamageable
 * 데미지를 입거나 피격 반응을 보여야 하는 모든 액터(몬스터, 플레이어, 파괴 오브젝트)가 상속받습니다.
 */
class FINALMINUTES_API IDamageable
{
    GENERATED_BODY()

public:
    /**
     * 피격 시 반응을 처리합니다. (이펙트, 사운드, 애니메이션 등)
     * BlueprintNativeEvent를 사용하여 C++에서 기본 로직을 만들고, 블루프린트에서 확장할 수 있게 합니다.
     * * @param HitResult 충돌 지점 및 노말 정보를 담고 있는 구조체
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
    void OnHitReaction(const FHitResult& HitResult);
};
