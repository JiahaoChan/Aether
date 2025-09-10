/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherWeatherEvent_Rainy.h"

#include "Distributions/DistributionFloatConstant.h"
#include "Distributions/DistributionFloatConstantCurve.h"
#include "Kismet/KismetMathLibrary.h"

#include "AetherAreaController.h"
#include "AetherWeatherEvent_Lightning.h"

UAetherWeatherEvent_Rainy::UAetherWeatherEvent_Rainy()
{
	EventType = EWeatherEventType::Rainy;
	
	RainFallMax = 0.0f;
	RainFallMin = 0.0f;
	
	OptionalLightningEvent = nullptr;
}

UAetherWeatherEventInstance* UAetherWeatherEvent_Rainy::MakeInstance_Native(AAetherAreaController* Outer)
{
	UAetherWeatherEventInstance_Rainy* Instance = NewObject<UAetherWeatherEventInstance_Rainy>(Outer);
	Instance->ContributedRainFall = 0.0f;
	Instance->PendingContributeRainFall = UKismetMathLibrary::RandomFloatInRangeFromStream(UKismetMathLibrary::MakeRandomStream(0), RainFallMin, RainFallMax);
	return Instance;
}

TArray<FWeatherEventDescription> UAetherWeatherEvent_Rainy::GetInnerWeatherEventDescriptions()
{
	TArray<FWeatherEventDescription> Result;
	if (OptionalLightningEvent)
	{
		FWeatherEventDescription NewDescription;
		NewDescription.Event = OptionalLightningEvent;
		NewDescription.TriggerSource = EWeatherTriggerSource::WeatherEvent;
		Result.Add(NewDescription);
	}
	return Result;
}

#if WITH_EDITOR
void UAetherWeatherEvent_Rainy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
	const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;
	
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAetherWeatherEvent_Rainy, BlendingInRainFall))
	{
		BlendingInRainFallCurve.Reset();
		if (UDistributionFloatConstantCurve* DistributionBlendingInRainFall = Cast<UDistributionFloatConstantCurve>(BlendingInRainFall.Distribution))
		{
			for (int32 i = 0; i < DistributionBlendingInRainFall->GetNumKeys(); i++)
			{
				BlendingInRainFallCurve.AddKey(DistributionBlendingInRainFall->GetKeyIn(i), DistributionBlendingInRainFall->GetKeyOut(0, i));
			}
		}
	}
}
#endif

void UAetherWeatherEvent_Rainy::PostInitProperties()
{
	Super::PostInitProperties();
	
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		UDistributionFloatConstantCurve* DistributionBlendingInRainFall = NewObject<UDistributionFloatConstantCurve>(this, TEXT("DistributionBlendingInRainFall"));
		DistributionBlendingInRainFall->ConstantCurve.AddPoint(0.0f, 0.0f);
		DistributionBlendingInRainFall->ConstantCurve.AddPoint(1.0f, 1.0f);
		BlendingInRainFall.Distribution = DistributionBlendingInRainFall;
		
		UDistributionFloatConstant* DistributionRunningRainFall = NewObject<UDistributionFloatConstant>(this, TEXT("DistributionRunningRainFall"));
		DistributionRunningRainFall->Constant = 0.0f;
		RunningRainFall.Distribution = DistributionRunningRainFall;
		
		UDistributionFloatConstantCurve* DistributionBlendingOutRainFall = NewObject<UDistributionFloatConstantCurve>(this, TEXT("DistributionBlendingOutRainFall"));
		DistributionBlendingOutRainFall->ConstantCurve.AddPoint(0.0f, 0.0f);
		DistributionBlendingOutRainFall->ConstantCurve.AddPoint(1.0f, 1.0f);
		BlendingOutRainFall.Distribution = DistributionBlendingOutRainFall;
	}
}

EWeatherEventExecuteState UAetherWeatherEventInstance_Rainy::BlendIn_Implementation(float DeltaTime, AAetherAreaController* AetherController)
{
	check(AetherController);
	if (!AetherController)
	{
		return EWeatherEventExecuteState::BlendingIn;
	}
	
	if (AetherController->GetCurrentState().AirTemperature < -5.0f)
	{
		return EWeatherEventExecuteState::BlendingOut;
	}
	
	float ThisFrameRainFall = FMath::Lerp(0.0f, PendingContributeRainFall, FMath::Clamp((CurrentStateLastTime + DeltaTime) / BlendInTime, 0.0f, 1.0f));
	float LastFrameRainFall = FMath::Lerp(0.0f, PendingContributeRainFall, FMath::Clamp(CurrentStateLastTime / BlendInTime, 0.0f, 1.0f));
	float Offset = ThisFrameRainFall - LastFrameRainFall;
	AetherController->GetCurrentState().RainFall += Offset;
	ContributedRainFall += Offset;
	return EWeatherEventExecuteState::BlendingIn;
}

EWeatherEventExecuteState UAetherWeatherEventInstance_Rainy::Run_Implementation(float DeltaTime, AAetherAreaController* AetherController)
{
	check(AetherController);
	
	if (AetherController->GetCurrentState().AirTemperature < -5.0f)
	{
		return EWeatherEventExecuteState::BlendingOut;
	}
	
	return EWeatherEventExecuteState::Running;
}

EWeatherEventExecuteState UAetherWeatherEventInstance_Rainy::BlendOut_Implementation(float DeltaTime, AAetherAreaController* AetherController)
{
	check(AetherController);
	if (!AetherController)
	{
		return EWeatherEventExecuteState::BlendingOut;
	}
	
	float ThisFrameRainFall = FMath::Lerp(ContributedRainFall, 0.0f, FMath::Clamp((CurrentStateLastTime + DeltaTime) / BlendOutTime, 0.0f, 1.0f));
	float LastFrameRainFall = FMath::Lerp(ContributedRainFall, 0.0f, FMath::Clamp(CurrentStateLastTime / BlendOutTime, 0.0f, 1.0f));
	float Offset = ThisFrameRainFall - LastFrameRainFall;
	AetherController->GetCurrentState().RainFall += Offset;
	return EWeatherEventExecuteState::BlendingOut;
}