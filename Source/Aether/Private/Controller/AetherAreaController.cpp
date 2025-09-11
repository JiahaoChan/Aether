/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherAreaController.h"

#include "Components/BillboardComponent.h"

#include "AetherStats.h"
#include "AetherWeatherEvent.h"
#include "AetherWorldSubsystem.h"
#include "AetherSystemPreset.h"

AAetherAreaController::AAetherAreaController()
{
#if WITH_EDITORONLY_DATA
	UBillboardComponent* SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	
	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTextureObject;
			FName ID_Aether;
			FText NAME_Aether;
			FConstructorStatics()
				: SpriteTextureObject(TEXT("/Aether/Icons/S_ParticleSystem"))
				, ID_Aether(TEXT("Aether"))
				, NAME_Aether(NSLOCTEXT("SpriteCategory", "Aether", "Aether"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;
		
		if (SpriteComponent)
		{
			SpriteComponent->Sprite = ConstructorStatics.SpriteTextureObject.Get();
			SpriteComponent->SetRelativeScale3D_Direct(FVector(0.5f, 0.5f, 0.5f));
			SpriteComponent->bHiddenInGame = true;
			SpriteComponent->bIsScreenSizeScaled = true;
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Aether;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Aether;
			SpriteComponent->SetupAttachment(RootComponent);
			SpriteComponent->bReceivesDecals = false;
		}
	}
#endif // WITH_EDITORONLY_DATA
	
#if WITH_EDITORONLY_DATA
	VisualizeComponent = CreateDefaultSubobject<UAetherAreaControllerVisualizeComponent>(TEXT("VisualizeComponent"));
#endif
	
	AffectRadius = 1000.0f;
	
#if WITH_EDITORONLY_DATA
	EarthLocationPreset = nullptr;
#endif
	
	DaytimeSpeedScale = 1.0f;
	NightSpeedScale = 1.0f;
	
	ActiveWeatherTags = FGameplayTagContainer::EmptyContainer;
	BlockingWeatherTags = FGameplayTagContainer::EmptyContainer;
	
	EvaporationCapacity = 500.0f;
	SurfaceWater = 0.0f;
	
	InitWeatherEventTags = FGameplayTagContainer::EmptyContainer;
	
	SinceLastTickTime = 0.0;
}

#if WITH_EDITOR
void AAetherAreaController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
	const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;
	
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, EarthLocationPreset))
	{
		if (EarthLocationPreset)
		{
			PossibleWeatherEvents.Empty();
			SubWeatherEvents.Empty();
			for (FWeatherEventDescription& Description : EarthLocationPreset->PossibleWeatherEvents)
			{
				if (Description.Event)
				{
					PossibleWeatherEvents.AddUnique(Description);
					for (FWeatherEventDescription& InnerDescription : Description.Event->GetInnerWeatherEventDescriptions())
					{
						if (InnerDescription.Event && InnerDescription.TriggerSource == EWeatherTriggerSource::WeatherEvent)
						{
							SubWeatherEvents.AddUnique(InnerDescription);
						}
					}
				}
			}
			for (FWeatherEventDescription& Description : PossibleWeatherEvents)
			{
				Description.ClampProbability();
				Description.UpdateHappeningMonthDisplayString();
			}
			EarthLocationPreset = nullptr;
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, DaytimeSpeedScale) || MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, NightSpeedScale))
	{
		SyncOtherControllerDielRhythm();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, PossibleWeatherEvents))
	{
		// Todo: Inner Weather Event
		for (FWeatherEventDescription& Description : PossibleWeatherEvents)
		{
			Description.ClampProbability();
			Description.UpdateHappeningMonthDisplayString();
		}
	}
}

bool AAetherAreaController::CanEditChange(const FProperty* InProperty) const
{
	return Super::CanEditChange(InProperty);
}
#endif

void AAetherAreaController::PostLoad()
{
	Super::PostLoad();
	
#if WITH_EDITORONLY_DATA
	for (FWeatherEventDescription& Description : PossibleWeatherEvents)
	{
		Description.UpdateHappeningMonthDisplayString();
	}
#endif
}

#if UE_ENABLE_DEBUG_DRAWING
void AAetherAreaController::DrawDebugPointInfo(const FColor& Color) const
{
	DrawDebugString(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, 30.0f), GetActorLabel(), nullptr, Color, 0, true);
	DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, 10.0f), 10.0f, 8, Color, false, 0);
	DrawDebugCylinder(GetWorld(), GetActorLocation(), GetActorLocation() + FVector(0.0f, 0.0f, FMath::Max(AffectRadius * 3.0f, 1000.0f)), AffectRadius, 16, Color, false, 0);
}
#endif

void AAetherAreaController::InitializeController()
{
	ActiveWeatherInstance.Reset();
	ActiveWeatherTags.Reset();
	BlockingWeatherTags.Reset();
	SinceLastTickTime = 0.0f;
	
	SurfaceWater = 0.0f;
	
	TestCount = 0;
}

void AAetherAreaController::TickAetherController(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_AetherController_Tick);
	
	// Cache first.
	LastState = CurrentState;
	
	float ActualDeltaTime = DeltaTime;
	SinceLastTickTime = DeltaTime;
	
#if WITH_EDITORONLY_DATA
	const UWorld * World = GetWorld();
	//if (World && (bSimulateInEditor && World->WorldType == EWorldType::Type::Editor || bSimulateInGame && World->IsGameWorld()))
#else
	//if (bSimulationInGame)
#endif
	{
		CalcControllerState_DielRhythm(ActualDeltaTime);
		
        EvaluateWeatherEvent(ActualDeltaTime);
        
        UpdateWeatherEvent(ActualDeltaTime);
		
		CalcSurfaceCoeffcient(ActualDeltaTime);
	}
}

void AAetherAreaController::CalcControllerState_DielRhythm(float DeltaTime)
{
	CalcSystemState_DielRhythm_Earth(DeltaTime);
}

void AAetherAreaController::CalcSystemState_DielRhythm_Earth(float DeltaTime)
{
	/*
	float DielDeltaTime = DeltaTime * (CurrentState.SunLightDirection.Z < 0.0f ? DaytimeSpeedScale : NightSpeedScale);
	CurrentState.ProgressOfYear += DielDeltaTime / (PeriodOfDay * DaysOfMonth * 12);
	CurrentState.ProgressOfYear = FMath::Frac(CurrentState.ProgressOfYear);
	
	CurrentState.Time += DeltaTime;
	
	UpdateSunByTime();
	*/
}

void AAetherAreaController::UpdateSunByTime()
{
	/*
	int32 PolarCondition = 0;
	float SunElevation = 0.0f;
	float SunAzimuth = 0.0f;
	const float ProgressOfDay = FMath::Frac(CurrentState.ProgressOfYear * DaysOfMonth * 12);
	CalculateSunPosition(
		Latitude,
		Longitude,
		ProgressOfDay * SECONDS_PER_DAY_EARTH,
		CurrentState.ProgressOfYear * SECONDS_PER_YEAR_EARTH,
		SunElevation,
		SunAzimuth,
		PolarCondition);
	
	CurrentState.SunElevation = SunElevation;
	CurrentState.SunAzimuth = SunAzimuth;
	CurrentState.SunLightDirection = ConvertPlanetLightDirection(SunElevation, SunAzimuth);
	*/
}

void AAetherAreaController::CalcSystemState_DielRhythm_Custom(float DeltaTime)
{
	
}

void AAetherAreaController::EvaluateWeatherEvent(float DeltaTime)
{
	TArray<int32> DeferredTriggerEventIndexList;
	for (int32 i = 0; i < PossibleWeatherEvents.Num(); i++)
	{
		const FWeatherEventDescription& EventDescription = PossibleWeatherEvents[i];
		if (EventDescription.TriggerSource != EWeatherTriggerSource::AetherController || !EventDescription.Event)
		{
			continue;
		}
		if (EventDescription.Event->EventTag.HasAnyExact(ActiveWeatherTags))
		{
			// Never trigger the same weather event twice in a period of time.
			continue;
		}
		if (EventDescription.Event->EventTag.HasAny(BlockingWeatherTags))
		{
			// Check current running event if blocks the incoming event.
			continue;
		}
		if (EventDescription.HappeningMonthsProbability.FindRef(CurrentState.Month, 0.0f) <= UE_KINDA_SMALL_NUMBER)
		{
			continue;
		}
		DeferredTriggerEventIndexList.Add(i);
	}
	
	// Check if blocking or canceling each other.
	TArray<int32> TriggerEventIndexList;
	for (const int32& Index: DeferredTriggerEventIndexList)
	{
		check(PossibleWeatherEvents.IsValidIndex(Index));
		const FWeatherEventDescription& EventDescription = PossibleWeatherEvents[Index];
		bool bShouldTrigger = true;
		for (const int32& OtherIndex: DeferredTriggerEventIndexList)
		{
			check(PossibleWeatherEvents.IsValidIndex(OtherIndex));
			if (Index != OtherIndex)
			{
				if (EventDescription.Event.Get()->EventTag.HasAny(PossibleWeatherEvents[OtherIndex].Event->BlockWeatherEventsWithTag))
				{
					bShouldTrigger = false;
					break;
				}
				if (EventDescription.Event.Get()->EventTag.HasAny(PossibleWeatherEvents[OtherIndex].Event->CancelWeatherEventsWithTag))
				{
					bShouldTrigger = false;
					break;
				}
			}
		}
		if (bShouldTrigger)
		{
			TriggerEventIndexList.Add(Index);
		}
	}
	DeferredTriggerEventIndexList.Reset();
	
	FGameplayTagContainer DeferredCancelEventTags;
	for (const int32& Index: TriggerEventIndexList)
	{
		check(PossibleWeatherEvents.IsValidIndex(Index));
		const FWeatherEventDescription& EventDescription = PossibleWeatherEvents[Index];
		DeferredCancelEventTags.AppendTags(EventDescription.Event->CancelWeatherEventsWithTag);
	}
	
	// Cancel
	TArray<UAetherWeatherEventInstance*> DeferredCancelEventInstances;
	for (UAetherWeatherEventInstance* Instance : ActiveWeatherInstance)
	{
		if (Instance && Instance->State == EWeatherEventExecuteState::Running)
		{
			check(Instance->EventClass);
			if (Instance->EventClass->EventTag.HasAny(DeferredCancelEventTags))
			{
				DeferredCancelEventInstances.Add(Instance);
			}
		}
	}
	for (UAetherWeatherEventInstance* Instance : DeferredCancelEventInstances)
	{
		if (Instance)
		{
			CancelWeatherEventImmediately(Instance);
		}
	}
	
	// Try to trigger
	for (const int32& Index : TriggerEventIndexList)
	{
		if (TestCount == 0)
		{
			check(PossibleWeatherEvents.IsValidIndex(Index));
            UAetherWeatherEventInstance* NewInstance = PossibleWeatherEvents[Index].Event->MakeInstance_Route(this);
            NewInstance->State = EWeatherEventExecuteState::JustSpawned;
            NewInstance->CurrentStateLastTime = 0.0f;
            ActiveWeatherInstance.Add(NewInstance);
			
			TestCount = 1;
		}
	}
	
	// Kick out finished instance.
	for (int32 i = ActiveWeatherInstance.Num() - 1; i >= 0; i--)
	{
		if (ActiveWeatherInstance[i] && ActiveWeatherInstance[i]->State == EWeatherEventExecuteState::Finished)
		{
			ActiveWeatherInstance.RemoveAt(i);
		}
	}
}

void AAetherAreaController::UpdateWeatherEvent(float DeltaTime)
{
	for (UAetherWeatherEventInstance* Instance : ActiveWeatherInstance)
	{
		if (Instance)
		{
			check(Instance->EventClass);
			const EWeatherEventExecuteState LastExecuteState = Instance->State;
            switch (LastExecuteState)
            {
                case EWeatherEventExecuteState::JustSpawned:
                	{
                		if (Instance->EventClass->DurationType == EWeatherEventDuration::Duration)
                		{
                			SetWeatherInstanceState(Instance, EWeatherEventExecuteState::BlendingIn);
                		}
		                else
		                {
			                SetWeatherInstanceState(Instance, EWeatherEventExecuteState::Running);
		                }
                		break;
                	}
                case EWeatherEventExecuteState::BlendingIn:
                	{
                		const EWeatherEventExecuteState NewState = Instance->BlendIn(DeltaTime, this);
                		if (LastExecuteState != NewState)
                		{
                			SetWeatherInstanceState(Instance, NewState);
                		}
                		else
                		{
                			Instance->CurrentStateLastTime += DeltaTime;
                			if (Instance->CurrentStateLastTime > Instance->BlendInTime)
                			{
                				SetWeatherInstanceState(Instance, EWeatherEventExecuteState::Running);
                			}
                		}
                		break;
                	}
                case EWeatherEventExecuteState::Running:
                	{
                		const EWeatherEventExecuteState NewState = Instance->Run(DeltaTime, this);
                		if (Instance->EventClass->DurationType == EWeatherEventDuration::Duration)
                		{
                			if (LastExecuteState != NewState)
                            {
                                SetWeatherInstanceState(Instance, NewState);
                            }
                            else
                            {
                                Instance->CurrentStateLastTime += DeltaTime;
                                if (Instance->CurrentStateLastTime > Instance->Duration)
                                {
                                    SetWeatherInstanceState(Instance, EWeatherEventExecuteState::BlendingOut);
                                }
                            }
                		}
		                else
		                {
			                SetWeatherInstanceState(Instance, EWeatherEventExecuteState::Finished);
		                }
                		break;
                	}
                case EWeatherEventExecuteState::BlendingOut:
                	{
                		const EWeatherEventExecuteState NewState = Instance->BlendOut(DeltaTime, this);
                		if (LastExecuteState != NewState)
                		{
                			SetWeatherInstanceState(Instance, NewState);
                		}
                		else
                		{
                			Instance->CurrentStateLastTime += DeltaTime;
                			if (Instance->CurrentStateLastTime > Instance->BlendOutTime)
                			{
                				SetWeatherInstanceState(Instance, EWeatherEventExecuteState::Finished);
                			}
                		}
                		break;
                	}
                default:
                	check(false);
                	break;
            }
		}
	}
}

void AAetherAreaController::TriggerWeatherEventImmediately(const FGameplayTag& EventTag)
{
	
}

void AAetherAreaController::TriggerWeatherEventImmediately(const FGameplayTagContainer& EventTags)
{
	
}

void AAetherAreaController::TriggerWeatherEventImmediately(const UAetherWeatherEvent* EventClass)
{
	
}

void AAetherAreaController::CancelWeatherEventImmediately(const FGameplayTag& EventTag)
{
	for (UAetherWeatherEventInstance* Instance : ActiveWeatherInstance)
	{
		check(Instance->EventClass);
		if (Instance && Instance->EventClass->EventTag.HasTag(EventTag))
		{
			CancelWeatherEventImmediately(Instance);
		}
	}
}

void AAetherAreaController::CancelWeatherEventImmediately(const FGameplayTagContainer& EventTags)
{
	for (UAetherWeatherEventInstance* Instance : ActiveWeatherInstance)
	{
		check(Instance->EventClass);
		if (Instance && Instance->EventClass->EventTag.HasAny(EventTags))
		{
			CancelWeatherEventImmediately(Instance);
		}
	}
}

void AAetherAreaController::CancelWeatherEventImmediately(const UAetherWeatherEvent* EventClass)
{
	for (UAetherWeatherEventInstance* Instance : ActiveWeatherInstance)
	{
		check(Instance->EventClass);
		if (Instance && Instance->EventClass == EventClass)
		{
			CancelWeatherEventImmediately(Instance);
		}
	}
}

void AAetherAreaController::CancelWeatherEventImmediately(UAetherWeatherEventInstance* EventInstance)
{
	if (!EventInstance)
	{
		return;
	}
	if (EventInstance->State != EWeatherEventExecuteState::Finished)
	{
		// Set to be Blend out would finish automatically.
		SetWeatherInstanceState(EventInstance, EWeatherEventExecuteState::BlendingOut);
	}
}

void AAetherAreaController::SetWeatherInstanceState(UAetherWeatherEventInstance* InInstance, const EWeatherEventExecuteState& NewState)
{
	if (!InInstance || InInstance->State == NewState)
	{
		return;
	}
	check(InInstance->EventClass);
	if (InInstance->State == EWeatherEventExecuteState::JustSpawned && NewState != EWeatherEventExecuteState::Finished)
	{
		ActiveWeatherTags.AppendTags(InInstance->EventClass->EventTag);
		check(InInstance->EventClass)
		// Instant Weather Event is too fast to block other event.
		if (InInstance->EventClass->DurationType == EWeatherEventDuration::Duration)
		{
			BlockingWeatherTags.AppendTags(InInstance->EventClass->BlockWeatherEventsWithTag);
		}
	}
	else if (NewState == EWeatherEventExecuteState::Finished)
	{
		ActiveWeatherTags.RemoveTags(InInstance->EventClass->EventTag);
		check(InInstance->EventClass)
		// Instant Weather Event is too fast to block other event.
		if (InInstance->EventClass->DurationType == EWeatherEventDuration::Duration)
		{
			BlockingWeatherTags.RemoveTags(InInstance->EventClass->BlockWeatherEventsWithTag);
		}
	}
	InInstance->State = NewState;
	InInstance->CurrentStateLastTime = 0.0f;
}

void AAetherAreaController::CalcSurfaceCoeffcient(float DeltaTime)
{
	check(CurrentState.RainFall >= 0.0f);
	if (SurfaceWater < 1000.0f)
	{
		SurfaceWater += CurrentState.RainFall * DeltaTime;
	}
	
	if (SurfaceWater > 0.0f)
	{
		SurfaceWater -= EvaporationCapacity * DeltaTime;
		SurfaceWater = FMath::Max(SurfaceWater, 0.0f);
	}
	CurrentState.TestValue = SurfaceWater;
	CurrentState.SurfaceRainRemain = FMath::Clamp(SurfaceWater / 100.0f, 0.0f, 1.0f);
	CurrentState.PuddleRainRemain = CurrentState.SurfaceRainRemain;
}

#if WITH_EDITOR
void AAetherAreaController::SyncOtherControllerDielRhythm()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		//Subsystem->SyncOtherControllerDielRhythm_Editor(this);
	}
}

void AAetherAreaController::CorrectOtherControllerInitTimeStamp()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		//Subsystem->CorrectOtherControllerInitTimeStamp_Editor(this);
	}
}
#endif