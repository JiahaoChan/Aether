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

void UAetherWeatherEventInstance::ConsumeEvent_Native(float DeltaTime, AAetherAreaController* AetherController)
{
	check(EventClass);
	check(AetherController);
	
	CurrentStateLastTime += DeltaTime;
	switch (State)
	{
		case EWeatherEventExecuteState::JustSpawned:
		{
			//if (AetherController->GetCurrentState().RunningWeatherTags.HasAllExact(EventClass->PreConditionWeatherEventsWithTag))
			{
				//SetState(EWeatherEventExecuteState::BlendingIn, AetherController);
			}
			//else
			{
				//for (const FGameplayTag& Event : EventClass->PreConditionWeatherEventsWithTag)
				{
					//AetherController->TriggerWeatherEventImmediately(Event);
				}
				//SetState(EWeatherEventExecuteState::Warming, AetherController);
			}
			break;
		}
		case EWeatherEventExecuteState::Warming:
		{
			// Todo
			//if (AetherController->GetCurrentState().RunningWeatherTags.HasAllExact(EventClass->PreConditionWeatherEventsWithTag))
			{
				//SetState(EWeatherEventExecuteState::BlendingIn, AetherController);
			}
			break;
		}
		default:
			break;
	}
	
	ConsumeEvent_Implementation(DeltaTime, AetherController);
}

void UAetherWeatherEventInstance::ConsumeEvent_Implementation(float DeltaTime, AAetherAreaController* AetherController)
{
	
}

void UAetherWeatherEventInstance::SetState(EWeatherEventExecuteState NewState, AAetherAreaController* AetherController)
{
	check(AetherController);
	if (State != NewState)
	{
		if (State == EWeatherEventExecuteState::Running)
		{
			if (NewState == EWeatherEventExecuteState::BlendingOut || NewState == EWeatherEventExecuteState::Finished)
			{
				OnInstanceEndRunning(AetherController);
			}
			else
			{
				// Todo
				checkf(false, TEXT("Invalid New State."));
			}
		}
		else if (State == EWeatherEventExecuteState::Warming || State == EWeatherEventExecuteState::BlendingIn)
		{
			if (NewState == EWeatherEventExecuteState::Running)
			{
				OnInstanceBeginRunning(AetherController);
			}
		}
		
		State = NewState;
		CurrentStateLastTime = 0.0f;
	}
}

void UAetherWeatherEventInstance::Cancel(AAetherAreaController* AetherController)
{
	SetState(EWeatherEventExecuteState::BlendingOut, AetherController);
}

void UAetherWeatherEventInstance::OnInstanceBeginRunning(AAetherAreaController* AetherController)
{
	check(EventClass);
	check(AetherController);
	//AetherController->RunningWeatherTags.AddTag(EventClass->EventTag);
}

void UAetherWeatherEventInstance::OnInstanceEndRunning(AAetherAreaController* AetherController)
{
	check(EventClass);
	check(AetherController);
	//AetherController->RunningWeatherTags.RemoveTag(EventClass->EventTag);
}