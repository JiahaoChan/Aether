/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherTypes.h"

FAetherState::FAetherState()
{
	Latitude = 0.0f;
	Longitude = 0.0f;
	ProgressOfYear = 0.0f;
	Month = EAetherMonth::January;
	SunLightDirection = FVector::ZeroVector;
	MoonLightDirection = FVector::ZeroVector;
	AirTemperature = 0.0f;
	GroundTemperature = 0.0f;
	RainFall = 0.0f;
	SnowFall = 0.0f;
	SurfaceRainRemain = 0.0f;
	SurfaceSnowDepth = 0.0f;
	WindData = FVector4f::Zero();
	DustIntensity = 0.0f;
	FogIntensity = 0.0f;
	
	Time = 0.0;
	SunElevation = 0.0f;
	SunAzimuth = 0.0f;
}

FString FAetherState::ToString() const
{
    FString Result;
	Result += FString("Latitude: ") + FString::SanitizeFloat(Latitude) + "\n";
	Result += FString("Longitude: ") + FString::SanitizeFloat(Longitude) + "\n";
	Result += FString("ProgressOfYear: ") + FString::SanitizeFloat(ProgressOfYear) + "\n";
	Result += FString("SunLightDirection: ") + SunLightDirection.ToString() + "\n";
	Result += FString("MoonLightDirection: ") + MoonLightDirection.ToString() + "\n";
	Result += FString("AirTemperature: ") + FString::SanitizeFloat(AirTemperature) + "\n";
	Result += FString("GroundTemperature: ") + FString::SanitizeFloat(GroundTemperature) + "\n";
	Result += FString("RainFall: ") + FString::SanitizeFloat(RainFall) + "\n";
	Result += FString("SnowFall: ") + FString::SanitizeFloat(SnowFall) + "\n";
	Result += FString("SurfaceRainRemain: ") + FString::SanitizeFloat(SurfaceRainRemain) + "\n";
	Result += FString("SurfaceSnowDepth: ") + FString::SanitizeFloat(SurfaceSnowDepth) + "\n";
	Result += FString("WindData: ") + WindData.ToString() + "\n";
	Result += FString("DustIntensity: ") + FString::SanitizeFloat(DustIntensity) + "\n";
	Result += FString("FogIntensity: ") + FString::SanitizeFloat(FogIntensity) + "\n";
	
	Result += FString("Time: ") + FString::SanitizeFloat(Time) + "\n";
	Result += FString("SunElevation: ") + FString::SanitizeFloat(SunElevation) + "\n";
	Result += FString("SunAzimuth: ") + FString::SanitizeFloat(SunAzimuth) + "\n";
	return Result;
}

void FAetherState::Reset()
{
	Latitude = 0.0f;
	Longitude = 0.0f;
	ProgressOfYear = 0.0f;
	Month = EAetherMonth::January;
	SunLightDirection = FVector::ZeroVector;
	MoonLightDirection = FVector::ZeroVector;
	AirTemperature = 0.0f;
	GroundTemperature = 0.0f;
	RainFall = 0.0f;
	SnowFall = 0.0f;
	SurfaceRainRemain = 0.0f;
	SurfaceSnowDepth = 0.0f;
	WindData = FVector4f::Zero();
	DustIntensity = 0.0f;
	FogIntensity = 0.0f;
	
	Time = 0.0;
	SunElevation = 0.0f;
	SunAzimuth = 0.0f;
}

void FAetherState::Normalize()
{
	ProgressOfYear = FMath::Fmod(ProgressOfYear, 1.0f);
	SunLightDirection.Normalize();
	MoonLightDirection.Normalize();
}

FAetherState FAetherState::operator*(float Operand)
{
	FAetherState Result;
	Result.ProgressOfYear = ProgressOfYear * Operand;
	Result.SunLightDirection = SunLightDirection * Operand;
	Result.MoonLightDirection = MoonLightDirection * Operand;
	
	Result.Time = Time * Operand;
	Result.SunElevation = SunElevation * Operand;
	Result.SunAzimuth = SunAzimuth * Operand;
	return Result;
}

FAetherState FAetherState::operator+(const FAetherState& Another)
{
	FAetherState Result;
	Result.ProgressOfYear = ProgressOfYear + Another.ProgressOfYear;
	Result.SunLightDirection = SunLightDirection + Another.SunLightDirection;
	Result.MoonLightDirection = MoonLightDirection + Another.MoonLightDirection;
	Result.AirTemperature = AirTemperature + Another.AirTemperature;
	Result.GroundTemperature = GroundTemperature + Another.GroundTemperature;
	Result.RainFall = RainFall + Another.RainFall;
	Result.SnowFall = SnowFall + Another.SnowFall;
	Result.SurfaceRainRemain = SurfaceRainRemain + Another.SurfaceRainRemain;
	Result.SurfaceSnowDepth = SurfaceSnowDepth + Another.SurfaceSnowDepth;
	Result.WindData = WindData + Another.WindData;
	Result.DustIntensity = DustIntensity + Another.DustIntensity;
	Result.FogIntensity = FogIntensity + Another.FogIntensity;
	
	Result.Time = Time + Another.Time;
	Result.SunElevation = SunElevation + Another.SunElevation;
	Result.SunAzimuth = SunAzimuth + Another.SunAzimuth;
	return Result;
}