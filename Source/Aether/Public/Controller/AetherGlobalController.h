/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"

#include "AetherControllerBase.h"
#include "AetherTypes.h"

#include "AetherGlobalController.generated.h"

#if WITH_EDITORONLY_DATA
UCLASS(NotBlueprintable)
class UAetherGlobalControllerVisualizeComponent : public UActorComponent
{
	GENERATED_BODY()
};
#endif

UCLASS()
class AETHER_API AAetherGlobalController : public AAetherControllerBase
{
	GENERATED_BODY()
	
	friend class UAetherWorldSubsystem;
	
protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UAetherGlobalControllerVisualizeComponent> VisualizeComponent;
#endif
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings")
	bool bSimulateInGame;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings")
	bool bSimulateInEditor;
#endif
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings")
	ESimulatePlanetType SimulatePlanet;
	
	// 纬度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings", meta = (ForceUnits = "deg", EditCondition = "SimulatePlanet == ESimulatePlanetType::Earth", EditConditionHides))
	float Latitude;
	
	// 经度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings", meta = (ForceUnits = "deg", EditCondition = "SimulatePlanet == ESimulatePlanetType::Earth", EditConditionHides))
	float Longitude;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings", meta = (ForceUnits = "m", EditCondition = "SimulatePlanet == ESimulatePlanetType::Earth", EditConditionHides))
	float NorthDisPerDegreeLatitude;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings", meta = (ForceUnits = "m", EditCondition = "SimulatePlanet == ESimulatePlanetType::Earth", EditConditionHides))
	float EastDisPerDegreeLongitude;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings", meta = (ForceUnits = "s", EditCondition = "SimulatePlanet == ESimulatePlanetType::Earth", EditConditionHides, ClampMin = "1.0"))
	float PeriodOfDay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings", meta = (EditCondition = "SimulatePlanet == ESimulatePlanetType::Earth", EditConditionHides, ClampMin = "1"))
	int32 DaysOfMonth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings", meta = (ForceUnits = "deg", EditCondition = "SimulatePlanet == ESimulatePlanetType::Earth", EditConditionHides))
	float NorthDirectionYawOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Settings", meta = (ForceUnits = "s"))
	float InitTimeStampOfYear;
	
public:
	AAetherGlobalController();
	
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End UObject Interface
	
private:
#if WITH_EDITOR
	UFUNCTION(CallInEditor, Category = "Aether")
	void CaptureTimeStamp();
#endif
};