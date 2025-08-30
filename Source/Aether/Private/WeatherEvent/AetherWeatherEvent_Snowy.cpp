/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherWeatherEvent_Snowy.h"

#include "AetherAreaController.h"

UAetherWeatherEventInstance* UAetherWeatherEvent_Snowy::MakeInstance_Native(AAetherAreaController* Outer)
{
	UAetherWeatherEventInstance_Snowy* Instance = NewObject<UAetherWeatherEventInstance_Snowy>(Outer);
	Instance->EventClass = this;
	Instance->State = EWeatherEventExecuteState::JustSpawned;
	Instance->CurrentStateLastTime = 0.0f;
	return Instance;
}