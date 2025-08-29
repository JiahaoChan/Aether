/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "WeatherEvent/AetherWeatherEvent_Cloudy.h"

#include "AetherAreaController.h"

UAetherWeatherEventInstance* UAetherWeatherEvent_Cloudy::MakeInstance_Native(AAetherAreaController* Outer)
{
	UAetherWeatherEventInstance_Cloudy* Instance = NewObject<UAetherWeatherEventInstance_Cloudy>(Outer);
	Instance->EventClass = this;
	Instance->State = EWeatherEventExecuteState::JustSpawned;
	Instance->CurrentStateLastTime = 0.0f;
	return Instance;
}