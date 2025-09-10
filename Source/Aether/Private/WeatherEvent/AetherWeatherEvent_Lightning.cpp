/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherWeatherEvent_Lightning.h"

#include "AetherAreaController.h"

UAetherWeatherEventInstance* UAetherWeatherEvent_Lightning::MakeInstance_Native(AAetherAreaController* Outer)
{
	UAetherWeatherEventInstance_Lightning* Instance = NewObject<UAetherWeatherEventInstance_Lightning>(Outer);
	return Instance;
}