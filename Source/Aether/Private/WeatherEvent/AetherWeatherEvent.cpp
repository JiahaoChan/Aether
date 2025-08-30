/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherWeatherEvent.h"

#include "AetherAreaController.h"

UAetherWeatherEvent::UAetherWeatherEvent()
{
	EventTag = FGameplayTag::EmptyTag;
	CancelWeatherEventsWithTag = FGameplayTagContainer::EmptyContainer;
	BlockWeatherEventsWithTag = FGameplayTagContainer::EmptyContainer;
	EventRange = EWeatherEventRangeType::Global;
	EventType = EWeatherEventType::Custom;
	Priority = 0;
	DurationType = EWeatherEventDuration::Duration;
	DurationMax = 1.0f;
	DurationMin = -1.0f;
	BlendInTimeMax = 1.0f;
	BlendInTimeMin = -1.0f;
	BlendOutTimeMax = 1.0f;
	BlendOutTimeMin = -1.0f;
	
	bMakeInstanceHasBlueprintImpl = false;
	{
		auto IsImplementedInBlueprint = [](const UFunction* Func) -> bool
		{
			return Func && ensure(Func->GetOuter()) && Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
		};
		
		const UFunction* Function = GetClass()->FindFunctionByName(FName("K2_MakeInstance"));
		if (Function && (HasAnyFlags(RF_ClassDefaultObject) || Function->IsValidLowLevelFast()))
		{
			bMakeInstanceHasBlueprintImpl = IsImplementedInBlueprint(Function);
		}
	}
}

#if WITH_EDITOR
void UAetherWeatherEvent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

UAetherWeatherEventInstance* UAetherWeatherEvent::MakeInstance_Route(AAetherAreaController* Outer)
{
	if (bMakeInstanceHasBlueprintImpl)
	{
		return K2_MakeInstance(Outer);
	}
	return MakeInstance_Native(Outer);
}

EWeatherEventExecuteState UAetherWeatherEventInstance::BlendIn_Implementation(float DeltaTime, AAetherAreaController* AetherController)
{
	return EWeatherEventExecuteState::Running;
}

EWeatherEventExecuteState UAetherWeatherEventInstance::Run_Implementation(float DeltaTime, AAetherAreaController* AetherController)
{
	return EWeatherEventExecuteState::BlendingOut;
}

EWeatherEventExecuteState UAetherWeatherEventInstance::BlendOut_Implementation(float DeltaTime, AAetherAreaController* AetherController)
{
	return EWeatherEventExecuteState::Finished;
}