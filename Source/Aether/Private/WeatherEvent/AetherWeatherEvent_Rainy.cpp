/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "WeatherEvent/AetherWeatherEvent_Rainy.h"

#include "Kismet/KismetMathLibrary.h"

#include "AetherAreaController.h"

UAetherWeatherEventInstance* UAetherWeatherEvent_Rainy::MakeInstance_Native(AAetherAreaController* Outer)
{
	UAetherWeatherEventInstance_Rainy* Instance = NewObject<UAetherWeatherEventInstance_Rainy>(Outer);
	Instance->EventClass = this;
	Instance->State = EWeatherEventExecuteState::JustSpawned;
	Instance->CurrentStateLastTime = 0.0f;
	
	Instance->ContributedRainFall = 0.0f;
	Instance->PendingContributeRainFall = UKismetMathLibrary::RandomFloatInRangeFromStream(UKismetMathLibrary::MakeRandomStream(0), RainFallMin, RainFallMax);
	return Instance;
}

void UAetherWeatherEventInstance_Rainy::ConsumeEvent_Implementation(float DeltaTime, AAetherAreaController* AetherController)
{
	if (AetherController->GetCurrentState().AirTemperature < -5.0f)
	{
		SetState(EWeatherEventExecuteState::BlendingOut, AetherController);
	}
	
	if (State == EWeatherEventExecuteState::BlendingIn)
	{
		float ThisFrameRainFall = FMath::Lerp(0.0f, PendingContributeRainFall, FMath::Clamp(CurrentStateLastTime / BlendInTime, 0.0f, 1.0f));
		float LastFrameRainFall = FMath::Lerp(0.0f, PendingContributeRainFall, FMath::Clamp((CurrentStateLastTime - DeltaTime) / BlendInTime, 0.0f, 1.0f));
		float Offset = ThisFrameRainFall - LastFrameRainFall;
		AetherController->GetCurrentState().RainFall += Offset;
		ContributedRainFall += Offset;
		if (Offset <= 0.0f && CurrentStateLastTime > 0.0f)
		{
			SetState(EWeatherEventExecuteState::Running, AetherController);
		}
	}
	else if (State == EWeatherEventExecuteState::BlendingOut)
	{
		float ThisFrameRainFall = FMath::Lerp(ContributedRainFall, 0.0f, FMath::Clamp(CurrentStateLastTime / BlendOutTime, 0.0f, 1.0f));
		float LastFrameRainFall = FMath::Lerp(ContributedRainFall, 0.0f, FMath::Clamp((CurrentStateLastTime - DeltaTime) / BlendOutTime, 0.0f, 1.0f));
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