#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/NetSerialization.h"
#include "SkillSpec.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UMechanicHandler;
class UPackageMap;
class APoE2ProjectileBase;
class APoE2AreaEffectBase;
class APoE2MinionBase;

/**
 * Network-friendly key-value pair for custom parameters
 */
USTRUCT(BlueprintType)
struct POE2FRAMEWORK_API FCustomParam
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomParam")
    FName Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CustomParam")
    float Value;

    FCustomParam()
        : Key(NAME_None), Value(0.0f)
    {
    }

    FCustomParam(FName InKey, float InValue)
        : Key(InKey), Value(InValue)
    {
    }

    bool operator==(const FCustomParam& Other) const
    {
        return Key == Other.Key;
    }
};

/**
 * SkillSpec：技能合成快照（SkillDA + SupportDA + PassiveDA + TalentDA + ItemDA 的最终结果）
 * 仅包含执行所需的数值与引用，不包含表现逻辑。
 */
USTRUCT(BlueprintType)
struct POE2FRAMEWORK_API FSkillSpec
{
    GENERATED_BODY()

public:
    FSkillSpec()
    {
        SkillId = NAME_None;
        AbilityClass = nullptr;
        ProjectileClass = nullptr;
        AreaClass = nullptr;
        SummonClass = nullptr;
        FinalDamage = 0.f;
        DamageEffectClass = nullptr;
        Cooldown = 0.f;
        ResourceCost = 0.f;
        CastTime = 0.f;
        AreaRadius = 0.f;
        ProjectileSpeed = 0.f;
        MaxRange = 0.f;
        Lifetime = 0.f;
        SummonCount = 1;
    }

    // 版本号（用于网络兼容性）
    static constexpr uint8 SKILLSPEC_VERSION = 1;

    // 基础标识与绑定
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    FName SkillId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec", meta=(AllowAbstract=false))
    TSubclassOf<UGameplayAbility> AbilityClass;

    // 承载体相关
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec", meta=(AllowAbstract=false))
    TSubclassOf<APoE2ProjectileBase> ProjectileClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec", meta=(AllowAbstract=false))
    TSubclassOf<APoE2AreaEffectBase> AreaClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec", meta=(AllowAbstract=false))
    TSubclassOf<APoE2MinionBase> SummonClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec", meta=(ClampMin=1, ClampMax=10))
    int32 SummonCount;

    // 数值（最终合成结果）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    float FinalDamage;

    /** The Gameplay Effect to apply for dealing damage. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    float Cooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    float ResourceCost;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    float CastTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    float AreaRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    float ProjectileSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    float MaxRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    float Lifetime;

    // 标签与效果
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    FGameplayTagContainer SkillTags;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    TArray<TSubclassOf<UGameplayEffect>> AppliedEffects;

    // 机制处理器（插件化）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec", meta=(MustImplement="MechanicHandler", AllowAbstract=false))
    TArray<TSubclassOf<UObject>> MechanicHandlers;

    // 额外参数（自定义数据）- 使用网络友好的数组结构
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="SkillSpec")
    TArray<FCustomParam> CustomParams;

    // Helper methods to maintain TMap-like interface
    FORCEINLINE bool Contains(FName Key) const
    {
        return CustomParams.ContainsByPredicate([Key](const FCustomParam& Param) { return Param.Key == Key; });
    }

    FORCEINLINE float GetCustomParam(FName Key, float DefaultValue = 0.0f) const
    {
        const FCustomParam* Found = CustomParams.FindByPredicate([Key](const FCustomParam& Param) { return Param.Key == Key; });
        return Found ? Found->Value : DefaultValue;
    }

    FORCEINLINE void SetCustomParam(FName Key, float Value)
    {
        FCustomParam* Found = CustomParams.FindByPredicate([Key](const FCustomParam& Param) { return Param.Key == Key; });
        if (Found)
        {
            Found->Value = Value;
        }
        else
        {
            CustomParams.Add(FCustomParam(Key, Value));
        }
    }

    // Bracket operator for TMap-like access
    FORCEINLINE float operator[](FName Key) const
    {
        return GetCustomParam(Key);
    }

    // 网络序列化支持
    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FSkillSpec> : public TStructOpsTypeTraitsBase2<FSkillSpec>
{
    enum
    {
        WithNetSerializer = true,
    };
};