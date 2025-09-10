/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "AetherSystemPreset.generated.h"

UCLASS(MinimalAPI)
class UAetherSystemPreset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 纬度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Latitude;
	
	// 经度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Longitude;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<struct FWeatherEventDescription> PossibleWeatherEvents;
	
	UAetherSystemPreset()
	{
		Latitude = 0.0f;
		Longitude = 0.0f;
	}
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual void PostLoad() override;
};