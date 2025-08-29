/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "AetherPluginSettings.generated.h"

UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "Aether"))
class AETHER_API UAetherPluginSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Config, Category = "Aether")
	TSoftObjectPtr<UMaterialParameterCollection> SystemMaterialParameterCollection;
	
	UPROPERTY(EditDefaultsOnly, Config, Category = "Aether")
	float SystemTickMinInterval;
	
public:
	UAetherPluginSettings();
	
	virtual FName GetCategoryName() const;
};