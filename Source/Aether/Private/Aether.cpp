/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "Aether.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "FAetherModule"

#define RootToContentDir StyleSet->RootToContentDir

void FAetherModule::StartupModule()
{
	StyleSet = MakeShared<FSlateStyleSet>("AetherStyle");
	
	const FString PluginDir = IPluginManager::Get().FindPlugin("Aether")->GetBaseDir();
	StyleSet->SetContentRoot(PluginDir / TEXT("Content/Slate"));
	//StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	
	const FVector2D Icon8x8(8.0f, 8.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon64x64(40.0f, 40.0f);
	const FVector2D Icon128x128(128.0f, 128.0f);
	const FVector2D Icon256x256(256.0f, 256.0f);
	
	StyleSet->Set("ClassIcon.AetherAreaController", new IMAGE_BRUSH_SVG("Icons/Classes/NiagaraEmitter_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.AetherAreaController", new IMAGE_BRUSH_SVG("Icons/Classes/NiagaraEmitter_64", Icon64x64));
	
	StyleSet->Set("ClassIcon.AetherLightingAvatar", new IMAGE_BRUSH_SVG("Icons/Classes/NiagaraSystem_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.AetherLightingAvatar", new IMAGE_BRUSH_SVG("Icons/Classes/NiagaraSystem_64", Icon64x64));
	
	StyleSet->Set("ClassThumbnail.AetherWeatherEvent_Rainy", new IMAGE_BRUSH("Icons/Classes/AetherWeatherEventInstance_Rainy_256", Icon256x256));
	
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}

void FAetherModule::ShutdownModule()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(StyleSet->GetStyleSetName());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAetherModule, Aether)