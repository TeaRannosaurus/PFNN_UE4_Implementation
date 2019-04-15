#include "PFNNAnimationModule.h"

DEFINE_LOG_CATEGORY(PFNNAnimationModuleLog);

#define LOCTEXT_NAMESPACE "PFNNAnimationModule"

void PFNNAnimationModule::StartupModule()
{
	UE_LOG(PFNNAnimationModuleLog, Log, TEXT("PFNNAnimationModule module has started!"));
}

void PFNNAnimationModule::ShutdownModule()
{
	UE_LOG(PFNNAnimationModuleLog, Log, TEXT("PFNNAnimationModule module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(PFNNAnimationModule, PFNNAnimationModule)