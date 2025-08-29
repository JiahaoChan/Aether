/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "AetherWeatherEvent.h"

#include "AetherWeatherEvent_Cloudy.generated.h"

UCLASS(NotBlueprintable)
class UAetherWeatherEvent_Cloudy : public UAetherWeatherEvent
{
	GENERATED_BODY()
	
public:
	UAetherWeatherEvent_Cloudy()
	{
		EventType = EWeatherEventType::Cloudy;
	}
	
	virtual UAetherWeatherEventInstance* MakeInstance_Native(AAetherAreaController* Outer) override;
};

UCLASS(NotBlueprintable)
class UAetherWeatherEventInstance_Cloudy : public UAetherWeatherEventInstance
{
	GENERATED_BODY()
};