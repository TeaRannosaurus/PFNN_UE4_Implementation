#include "PFNNAnimationEditorModule.h"

DEFINE_LOG_CATEGORY(PFNNAnimationEditorModuleLog);

#define LOCTEXT_NAMESPACE "PFNNAnimationEditorModule"

void PFNNAnimationEditorModule::StartupModule()
{
	UE_LOG(PFNNAnimationEditorModuleLog, Log, TEXT("PFNNAnimationEditorModule module has started!"));
}

void PFNNAnimationEditorModule::ShutdownModule()
{
	UE_LOG(PFNNAnimationEditorModuleLog, Log, TEXT("PFNNAnimationEditorModule module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(PFNNAnimationEditorModule, PFNNAnimationEditor)