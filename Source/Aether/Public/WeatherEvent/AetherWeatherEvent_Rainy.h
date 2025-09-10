/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "Distributions/DistributionFloat.h"

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
	
	UPROPERTY(EditAnywhere, Category="Rainy")
	FRichCurve BlendingInRainFallCurve;
	
	UPROPERTY(EditDefaultsOnly, Category = "Rainy")
	FRawDistributionFloat BlendingInRainFall;
	
	UPROPERTY(EditDefaultsOnly, Category = "Rainy")
	FRawDistributionFloat RunningRainFall;
	
	UPROPERTY(EditDefaultsOnly, Category = "Rainy")
	FRawDistributionFloat BlendingOutRainFall;
	
	UPROPERTY(EditDefaultsOnly, Category = "Rainy")
	TObjectPtr<class UAetherWeatherEvent_Lightning> OptionalLightningEvent;
	
	UAetherWeatherEvent_Rainy();
	
	//~ Begin UAetherWeatherEvent Interface
	virtual UAetherWeatherEventInstance* MakeInstance_Native(AAetherAreaController* Outer) override;
	
	virtual TArray<FWeatherEventDescription> GetInnerWeatherEventDescriptions() override;
	//~ End UAetherWeatherEvent Interface
	
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostInitProperties() override;
	//~ End UObject Interface
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
	
	virtual EWeatherEventExecuteState BlendIn_Implementation(float DeltaTime, AAetherAreaController* AetherController) override;
	
	virtual EWeatherEventExecuteState Run_Implementation(float DeltaTime, AAetherAreaController* AetherController) override;
	
	virtual EWeatherEventExecuteState BlendOut_Implementation(float DeltaTime, AAetherAreaController* AetherController) override;
};