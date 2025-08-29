/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "AetherWeatherEvent.h"

#include "AetherWeatherEvent_Rainy.generated.h"

UCLASS(NotBlueprintable)
class UAetherWeatherEvent_Rainy : public UAetherWeatherEvent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rainy")
	float RainFallMax;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rainy")
	float RainFallMin;
	
	UAetherWeatherEvent_Rainy()
	{
		EventType = EWeatherEventType::Rainy;
		
		RainFallMax = 0.0f;
		RainFallMin = 0.0f;
	}
	
	virtual UAetherWeatherEventInstance* MakeInstance_Native(AAetherAreaController* Outer) override;
};

UCLASS(NotBlueprintable)
class UAetherWeatherEventInstance_Rainy : public UAetherWeatherEventInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	float ContributedRainFall;
	
	UPROPERTY()
	float PendingContributeRainFall;
	
	UAetherWeatherEventInstance_Rainy()
	{
		ContributedRainFall = 0.0f;
		PendingContributeRainFall = 0.0f;
	}
	
	void ConsumeEvent_Implementation(float DeltaTime, AAetherAreaController* AetherController) override;
};