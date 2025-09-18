#include "PoE2Framework.h"
#include "Core/PoE2Log.h"
#include "Core/PoE2Tags.h"

#define LOCTEXT_NAMESPACE "FPoE2FrameworkModule"

DEFINE_LOG_CATEGORY(LogPoE2Framework);

void FPoE2FrameworkModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file
    FPoE2Tags::InitializeNativeTags();

    UE_LOG(LogPoE2Framework, Warning, TEXT("PoE2Framework module has started!"));
}

void FPoE2FrameworkModule::ShutdownModule()
{
    UE_LOG(LogPoE2Framework, Warning, TEXT("PoE2Framework module has shut down"));
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FPoE2FrameworkModule, PoE2Framework)