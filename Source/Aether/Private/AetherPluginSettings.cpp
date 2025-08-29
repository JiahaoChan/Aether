/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherPluginSettings.h"

UAetherPluginSettings::UAetherPluginSettings()
{
	SystemMaterialParameterCollection = nullptr;
	SystemTickMinInterval = 0.013333f;
}

FName UAetherPluginSettings::GetCategoryName() const
{
    return FName(TEXT("Plugins"));
}