#pragma once

#include "CoreMinimal.h"
#include "GameplayCueInterface.h"
#include "CueParams.generated.h"

class AActor;
struct FSkillSpec;

USTRUCT(BlueprintType)
struct POE2FRAMEWORK_API FPoE2CueParams
{
    GENERATED_BODY()

public:
    FPoE2CueParams()
    {
        SourceActor = nullptr;
        TargetActor = nullptr;
        Location = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        SkillSpecPtr = nullptr;
        Magnitude = 1.0f;
        Duration = 0.0f;
    }

    // 源对象（施法者）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CueParams")
    TWeakObjectPtr<AActor> SourceActor;

    // 目标对象
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CueParams")
    TWeakObjectPtr<AActor> TargetActor;

    // 位置信息
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CueParams")
    FVector Location;

    // 速度信息（用于投掷物等）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CueParams")
    FVector Velocity;

    // 技能快照引用（弱指针，避免循环依赖）
    // Note: 不使用 UPROPERTY 因为 USTRUCT 指针不适合反射系统
    const FSkillSpec* SkillSpecPtr;

    // 强度/倍数
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CueParams")
    float Magnitude;

    // 持续时间
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CueParams")
    float Duration;

    // 自定义参数
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CueParams")
    TMap<FName, float> CustomParams;
};