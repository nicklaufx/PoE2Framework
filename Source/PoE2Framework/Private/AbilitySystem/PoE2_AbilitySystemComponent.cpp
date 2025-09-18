// Copyright Your Company, Inc. All Rights Reserved.

#include "AbilitySystem/PoE2_AbilitySystemComponent.h"
#include "Data/SkillDataAsset.h"
#include "Data/SupportDataAsset.h"

TArray<FPatch> UPoE2_AbilitySystemComponent::GetPatchesForSkill(const USkillDataAsset* SkillToFind) const
{
    TArray<FPatch> FoundPatches;

    if (!SkillToFind)
    {
        return FoundPatches;
    }

    // 1. 遍历所有已装备的技能
    for (const FActiveSkillLink& SkillLink : EquippedSkills)
    {
        // 2. 找到与传入技能匹配的那一项
        if (SkillLink.Skill == SkillToFind)
        {
            // 3. 遍历这个技能链接的所有辅助宝石
            for (const USupportDataAsset* SupportDA : SkillLink.LinkedSupports)
            {
                if (SupportDA)
                {
                    // 4. 将每个辅助宝石的 "SkillPatch" 添加到结果数组中
                    FoundPatches.Add(SupportDA->SkillPatch);
                }
            }
            // 找到了就没必要继续循环了
            break; 
        }
    }

    return FoundPatches;
}

void UPoE2_AbilitySystemComponent::EquipSkill(USkillDataAsset* NewSkill)
{
    if(NewSkill)
    {
        bool bAlreadyEquipped = false;
        for (const FActiveSkillLink& SkillLink : EquippedSkills)
        {
            if (SkillLink.Skill == NewSkill)
            {
                bAlreadyEquipped = true;
                break;
            }
        }

        if (!bAlreadyEquipped)
        {
            FActiveSkillLink NewLink;
            NewLink.Skill = NewSkill;
            EquippedSkills.Add(NewLink);
            
            if (NewSkill->AbilityClass)
            {
                FGameplayAbilitySpec Spec(NewSkill->AbilityClass, 1, -1, NewSkill);
                GiveAbility(Spec);
            }
        }
    }
}

void UPoE2_AbilitySystemComponent::LinkSupportToSkill(USupportDataAsset* Support, USkillDataAsset* TargetSkill)
{
    if(Support && TargetSkill)
    {
        for (FActiveSkillLink& SkillLink : EquippedSkills)
        {
            if (SkillLink.Skill == TargetSkill)
            {
                SkillLink.LinkedSupports.AddUnique(Support);
                break;
            }
        }
    }
}
