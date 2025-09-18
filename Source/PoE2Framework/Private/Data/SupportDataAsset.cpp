#include "Data/SupportDataAsset.h"
#include "Engine/AssetManager.h"

USupportDataAsset::USupportDataAsset()
{
    // Set default values
    SupportId = NAME_None;
    DisplayName = FText::GetEmpty();
}

FPrimaryAssetId USupportDataAsset::GetPrimaryAssetId() const
{
    // Return a FPrimaryAssetId with a type of 'Support' and a name of this asset's FName
    // This is important for the Asset Manager to discover and manage support gems
    return FPrimaryAssetId(TEXT("Support"), GetFName());
}