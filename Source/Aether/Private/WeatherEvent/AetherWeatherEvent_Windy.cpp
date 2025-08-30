/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherWeatherEvent_Windy.h"

#include "AetherAreaController.h"

UAetherWeatherEventInstance* UAetherWeatherEvent_Windy::MakeInstance_Native(AAetherAreaController* Outer)
{
	UAetherWeatherEventInstance_Windy* Instance = NewObject<UAetherWeatherEventInstance_Windy>(Outer);
	Instance->EventClass = this;
	Instance->State = EWeatherEventExecuteState::JustSpawned;
	Instance->CurrentStateLastTime = 0.0f;
	return Instance;
}