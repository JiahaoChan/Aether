/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherSystemPreset.h"

#include "UObject/ObjectSaveContext.h"

#include "AetherWeatherEvent.h"

#if WITH_EDITOR
void UAetherSystemPreset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
	const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;
	
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAetherSystemPreset, Latitude))
	{
		Latitude = FMath::Fmod(FMath::Fmod(Latitude + 90.0f, 180.0f) + 180.0 , 180.0f) - 90.0f;
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAetherSystemPreset, Longitude))
	{
		Longitude = FMath::Fmod(FMath::Fmod(Longitude + 180.0f, 360.0f) + 360.0 , 360.0f) - 180.0f;
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAetherSystemPreset, PossibleWeatherEvents))
	{
		for (FWeatherEventDescription& Description : PossibleWeatherEvents)
		{
			Description.UpdateHappeningMonthDisplayString();
			Description.FixProbability();
		}
	}
}

void UAetherSystemPreset::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
	
}
#endif

void UAetherSystemPreset::PostLoad()
{
	Super::PostLoad();
	
#if WITH_EDITORONLY_DATA
	for (FWeatherEventDescription& Description : PossibleWeatherEvents)
	{
		Description.UpdateHappeningMonthDisplayString();
		Description.FixProbability();
	}
#endif
}