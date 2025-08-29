/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "AetherWeatherEvent.h"

#include "AetherWeatherEvent_Windy.generated.h"

UCLASS(NotBlueprintable)
class UAetherWeatherEvent_Windy : public UAetherWeatherEvent
{
	GENERATED_BODY()
	
public:
	UAetherWeatherEvent_Windy()
	{
		EventType = EWeatherEventType::Windy;
	}
	
	virtual UAetherWeatherEventInstance* MakeInstance_Native(AAetherAreaController* Outer) override;
};

UCLASS(NotBlueprintable)
class UAetherWeatherEventInstance_Windy : public UAetherWeatherEventInstance
{
	GENERATED_BODY()
};