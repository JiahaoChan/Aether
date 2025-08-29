/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "WeatherEvent/AetherWeatherEvent_Snowy.h"

#include "AetherAreaController.h"

UAetherWeatherEventInstance* UAetherWeatherEvent_Snowy::MakeInstance_Native(AAetherAreaController* Outer)
{
	UAetherWeatherEventInstance_Snowy* Instance = NewObject<UAetherWeatherEventInstance_Snowy>(Outer);
	Instance->EventClass = this;
	Instance->State = EWeatherEventExecuteState::JustSpawned;
	Instance->CurrentStateLastTime = 0.0f;
	return Instance;
}

void UAetherWeatherEventInstance_Snowy::ConsumeEvent_Implementation(float DeltaTime, AAetherAreaController* AetherController)
{
	if (State == EWeatherEventExecuteState::BlendingIn)
	{
		float ThisFrameRainFall = FMath::Lerp(0.0f, PendingContributeSnowFall, FMath::Clamp(CurrentStateLastTime / BlendInTime, 0.0f, 1.0f));
		float LastFrameRainFall = FMath::Lerp(0.0f, PendingContributeSnowFall, FMath::Clamp((CurrentStateLastTime - DeltaTime) / BlendInTime, 0.0f, 1.0f));
		float Offset = ThisFrameRainFall - LastFrameRainFall;
		AetherController->GetCurrentState().RainFall += Offset;
		ContributedSnowFall += Offset;
		if (Offset <= 0.0f && CurrentStateLastTime > 0.0f)
		{
			SetState(EWeatherEventExecuteState::Running, AetherController);
		}
	}
	else if (State == EWeatherEventExecuteState::BlendingOut)
	{
		float ThisFrameRainFall = FMath::Lerp(ContributedSnowFall, 0.0f, FMath::Clamp(CurrentStateLastTime / BlendOutTime, 0.0f, 1.0f));
		float LastFrameRainFall = FMath::Lerp(ContributedSnowFall, 0.0f, FMath::Clamp((CurrentStateLastTime - DeltaTime) / BlendOutTime, 0.0f, 1.0f));
		float Offset = ThisFrameRainFall - LastFrameRainFall;
		AetherController->GetCurrentState().RainFall += Offset;
		if (Offset >= 0.0f && CurrentStateLastTime > 0.0f)
		{
			SetState(EWeatherEventExecuteState::Finished, AetherController);
		}
	}
	else if (State == EWeatherEventExecuteState::Running)
	{
		
	}
}