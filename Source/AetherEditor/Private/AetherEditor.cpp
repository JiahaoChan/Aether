/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherEditor.h"

#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

#include "AetherComponentVisualizer.h"

#include "AetherAreaController.h"
#include "AetherGlobalController.h"

#define LOCTEXT_NAMESPACE "FAetherEditorModule"

void FAetherEditorModule::StartupModule()
{
	RegisterComponentVisualizer(UAetherGlobalControllerVisualizeComponent::StaticClass()->GetFName(), MakeShareable(new FAetherGlobalControllerComponentVisualizer));
	RegisterComponentVisualizer(UAetherAreaControllerVisualizeComponent::StaticClass()->GetFName(), MakeShareable(new FAetherAreaControllerComponentVisualizer));
}

void FAetherEditorModule::ShutdownModule()
{
	
}

void FAetherEditorModule::RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer)
{
	if (GUnrealEd)
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
	}
	if (Visualizer.IsValid())
	{
		Visualizer->OnRegister();
	}
}
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAetherEditorModule, AetherEditor)