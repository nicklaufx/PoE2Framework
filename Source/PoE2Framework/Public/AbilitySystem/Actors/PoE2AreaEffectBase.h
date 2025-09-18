#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoE2AreaEffectBase.generated.h"

UCLASS(BlueprintType)
class POE2FRAMEWORK_API APoE2AreaEffectBase : public AActor
{
    GENERATED_BODY()

public:
    APoE2AreaEffectBase();

protected:
    // TODO: Define area effect properties with bReplicates = true
};