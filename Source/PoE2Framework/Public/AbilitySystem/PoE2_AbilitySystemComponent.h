// Copyright Your Company, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Data/Patch.h" // 需要包含 Patch.h
#include "PoE2_AbilitySystemComponent.generated.h"

class USkillDataAsset;
class USupportDataAsset;

// 用一个结构体来清晰地表示一个主动技能及其链接的辅助宝石
USTRUCT(BlueprintType)
struct FActiveSkillLink
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<USkillDataAsset> Skill;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TObjectPtr<USupportDataAsset>> LinkedSupports;
};

UCLASS()
class POE2FRAMEWORK_API UPoE2_AbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:
    // 用一个数组来存储所有已装备的主动技能
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Skills")
    TArray<FActiveSkillLink> EquippedSkills;

    /**
     * 核心函数：根据给定的技能DataAsset，查找所有链接的辅助宝石，并返回它们的Patch数组
     * @param SkillToFind 要查找的主动技能
     * @return 一个包含所有相关Patch的数组
     */
    UFUNCTION(BlueprintPure, Category="Skills")
    TArray<FPatch> GetPatchesForSkill(const USkillDataAsset* SkillToFind) const;
    
    UFUNCTION(BlueprintCallable, Category="Skills")
    void EquipSkill(USkillDataAsset* NewSkill);

    UFUNCTION(BlueprintCallable, Category="Skills")
    void LinkSupportToSkill(USupportDataAsset* Support, USkillDataAsset* TargetSkill);
};
