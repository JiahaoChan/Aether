/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherWeatherEvent_Cloudy.h"

#include "AetherAreaController.h"

UAetherWeatherEventInstance* UAetherWeatherEvent_Cloudy::MakeInstance_Native(AAetherAreaController* Outer)
{
	UAetherWeatherEventInstance_Cloudy* Instance = NewObject<UAetherWeatherEventInstance_Cloudy>(Outer);
	return Instance;
}