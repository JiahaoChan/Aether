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
enum class ESimulationPlanetType : uint8
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
	AetherController		UMETA(DisplayName = "Aether Controller"),
	External				UMETA(DisplayName = "External"),
};

USTRUCT(BlueprintType)
struct FAetherState
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProgressOfYear;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAetherMonth Month;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SunLightDirection;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MoonLightDirection;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirTemperature;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GroundTemperature;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RainFall;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SnowFall;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurfaceRainRemain;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurfaceSnowDepth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4f WindData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DustIntensity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogIntensity;
	
	float Time;
	float SunElevation;
	float SunAzimuth;
	
	FAetherState();
	
	FString ToString() const;
	
	void Reset();
	
	void Normalize();
	
	virtual FAetherState operator*(float Operand);
	
	virtual FAetherState operator+(const FAetherState& Another);
};