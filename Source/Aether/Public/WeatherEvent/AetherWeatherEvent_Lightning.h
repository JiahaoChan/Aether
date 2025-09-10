/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "AetherWeatherEvent.h"

#include "AetherWeatherEvent_Lightning.generated.h"

UCLASS(NotBlueprintable)
class UAetherWeatherEvent_Lightning : public UAetherWeatherEvent
{
	GENERATED_BODY()
	
public:
	UAetherWeatherEvent_Lightning()
	{
		EventType = EWeatherEventType::Lightning;
	}
	
	virtual UAetherWeatherEventInstance* MakeInstance_Native(AAetherAreaController* Outer) override;
};

UCLASS(NotBlueprintable)
class UAetherWeatherEventInstance_Lightning : public UAetherWeatherEventInstance
{
	GENERATED_BODY()
};