/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "AetherWeatherEvent.h"

#include "AetherWeatherEvent_Snowy.generated.h"

UCLASS(NotBlueprintable)
class UAetherWeatherEvent_Snowy : public UAetherWeatherEvent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snowy")
	float SnowFallMax;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Snowy")
	float SnowFallMin;
	
	UAetherWeatherEvent_Snowy()
	{
		EventType = EWeatherEventType::Snowy;
		
		SnowFallMax = 0.0f;
		SnowFallMin = 0.0f;
	}
	
	virtual UAetherWeatherEventInstance* MakeInstance_Native(AAetherAreaController* Outer) override;
};

UCLASS(NotBlueprintable)
class UAetherWeatherEventInstance_Snowy : public UAetherWeatherEventInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	float ContributedSnowFall;
	
	UPROPERTY()
	float PendingContributeSnowFall;
	
	UAetherWeatherEventInstance_Snowy()
	{
		ContributedSnowFall = 0.0f;
		PendingContributeSnowFall = 0.0f;
	}
	
	void ConsumeEvent_Implementation(float DeltaTime, AAetherAreaController* AetherController) override;
};