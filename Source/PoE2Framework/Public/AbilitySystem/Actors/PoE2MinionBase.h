#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PoE2MinionBase.generated.h"

UCLASS(BlueprintType)
class POE2FRAMEWORK_API APoE2MinionBase : public APawn
{
    GENERATED_BODY()

public:
    APoE2MinionBase();

protected:
    // TODO: Define minion properties with bReplicates = true
};