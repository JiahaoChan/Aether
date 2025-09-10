/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "AetherTypes.generated.h"

UENUM(BlueprintType)
enum class ESimulatePlanetType : uint8
{
	Earth			UMETA(DisplayName = "Earth"),
	CustomPlanet	UMETA(Hidden),
};

UENUM(BlueprintType)
enum class EAetherSeason: uint8
{
	Spring			UMETA(DisplayName = "Spring"),
	Summer			UMETA(DisplayName = "Summer"),
	Fall			UMETA(DisplayName = "Fall"),
	Winter			UMETA(DisplayName = "Winter"),
};

UENUM(BlueprintType)
enum class EAetherMonth: uint8
{
	January			UMETA(DisplayName = "January"),
	February		UMETA(DisplayName = "February"),
	March			UMETA(DisplayName = "March"),
	April			UMETA(DisplayName = "April"),
	May				UMETA(DisplayName = "May"),
	June			UMETA(DisplayName = "June"),
	July			UMETA(DisplayName = "July"),
	August			UMETA(DisplayName = "August"),
	September		UMETA(DisplayName = "September"),
	October			UMETA(DisplayName = "October"),
	November		UMETA(DisplayName = "November"),
	December		UMETA(DisplayName = "December"),
};

UENUM(BlueprintType)
enum class EWeatherTriggerSource: uint8
{
	/**
	 * Evaluated if happens by AetherAreaController.
	 */
	AetherController		UMETA(DisplayName = "Aether Controller"),
	/**
	 * Triggered by other weather events as a sub weather event.
	 */
	WeatherEvent			UMETA(DisplayName = "WeatherEvent"),
	GameplayExternal		UMETA(DisplayName = "GameplayExternal"),
};

UENUM(BlueprintType)
enum class EWeatherEventRangeType : uint8
{
	Global			UMETA(DisplayName = "Global"),
	Local			UMETA(DisplayName = "Local"),
};

UENUM(BlueprintType)
enum class EWeatherEventType : uint8
{
	Cloudy			UMETA(DisplayName = "Cloudy"),
	Rainy			UMETA(DisplayName = "Rainy"),
	Snowy			UMETA(DisplayName = "Snowy"),
	Windy			UMETA(DisplayName = "Windy"),
	Foggy			UMETA(DisplayName = "Foggy"),
	Lightning		UMETA(DisplayName = "Lightning"),
	Rainbow			UMETA(DisplayName = "Rainbow"),
	Custom			UMETA(DisplayName = "Custom"),
};

UENUM(BlueprintType)
enum class EWeatherEventExecuteState : uint8
{
	JustSpawned		UMETA(DisplayName = "JustSpawned"),
	BlendingIn		UMETA(DisplayName = "BlendigIn"),
	Running			UMETA(DisplayName = "Running"),
	BlendingOut		UMETA(DisplayName = "BlendigIn"),
	Finished		UMETA(DisplayName = "Finished"),
};

UENUM()
enum class EWeatherEventDuration : uint8
{
	Instant		UMETA(DisplayName = "Instant"),
	Duration	UMETA(DisplayName = "Duration"),
};

USTRUCT(BlueprintType)
struct FAetherState
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "deg"))
	float Latitude;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "deg"))
	float Longitude;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "%"))
	float ProgressOfYear;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAetherMonth Month;
	
	/**
	 * Vector Pivot: Planet, Vector Point at: Local Position.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SunLightDirection;
	
	/**
	 * Vector Pivot: Planet, Vector Point at: Local Position.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MoonLightDirection;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "C"))
	float AirTemperature;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "C"))
	float GroundTemperature;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "mm"))
	float RainFall;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "mm"))
	float SnowFall;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "%"))
	float SurfaceRainRemain;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "%"))
	float PuddleRainRemain;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "%"))
	float SurfaceSnowDepth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4f WindData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DustIntensity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogIntensity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ForceUnits = "%"))
	float CloudCoverage;
	
	float Time;
	float SunElevation;
	float SunAzimuth;
	float TestValue;
	
	FAetherState();
	
	FString ToString() const;
	
	void Reset();
	
	void Normalize();
	
	virtual FAetherState operator*(float Operand);
	
	virtual FAetherState operator+(const FAetherState& Another);
};