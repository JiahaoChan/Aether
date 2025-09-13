/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherWorldSubsystem.h"

#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

#include "AetherAreaController.h"
#include "AetherCloudAvatar.h"
#include "AetherControllerBase.h"
#include "AetherGlobalController.h"
#include "AetherLightingAvatar.h"
#include "AetherPluginSettings.h"
#include "AetherStats.h"

#include "AetherWorldMath.inl"

#if UE_ENABLE_DEBUG_DRAWING
static TAutoConsoleVariable<int32> CVarVisualizeAetherState(
	TEXT("a.VisualizeAetherState"),
	0,
	TEXT("Visualize the State of the Aether System."),
	ECVF_Cheat);

static TAutoConsoleVariable<int32> CVarVisualizeAetherControllerWeight(
	TEXT("a.VisualizeAetherControllerWeight"),
	0,
	TEXT("Visualize the distance-based weight of Aether Controllers."),
	ECVF_Cheat);

static TAutoConsoleVariable<int32> CVarVisualizeAetherControllerPoint(
	TEXT("a.VisualizeAetherControllerPoint"),
	0,
	TEXT("Visualize the location point of Aether Controllers."),
	ECVF_Cheat);
#endif

UAetherWorldSubsystem::UAetherWorldSubsystem()
{
	LightingAvatar = nullptr;
	CloudAvatar = nullptr;
	SystemMaterialParameterCollection = nullptr;
}

bool UAetherWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}
	TArray<UClass*> DerivedClassList;
	GetDerivedClasses(GetClass(), DerivedClassList, false);
	return DerivedClassList.Num() == 0;
}

void UAetherWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UAetherPluginSettings* Settings = GetDefault<UAetherPluginSettings>();
	
	if (UMaterialParameterCollection* ParameterCollection = Settings->SystemMaterialParameterCollection.LoadSynchronous())
	{
		SystemMaterialParameterCollection = ParameterCollection;
	}
	else
	{
		// Todo: Log
	}
	
	GlobalController = nullptr;
	AreaControllers.Empty();
	ActiveControllers.Empty();
	Avatars.Empty();
	LightingAvatar = nullptr;
	CloudAvatar = nullptr;
	SystemState.Reset();
	StreamingSourceLocation = FVector4f::Zero();
	StreamingSourceLocation.W = -1.0f;
	
#if WITH_EDITOR
	if (UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			FEditorDelegates::OnMapOpened.AddUObject(this, &UAetherWorldSubsystem::OnMapOpened);
		}
	}
#endif
}

bool UAetherWorldSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::Editor || WorldType == EWorldType::PIE;
}

bool UAetherWorldSubsystem::IsTickable() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
#if WITH_EDITOR
	if (World->WorldType == EWorldType::Type::Editor && GEditor)
	{
		if (const FViewport* Viewport = GEditor->GetActiveViewport())
		{
			// Editor world still maintains when pulling up a PIE world, but it can not be ever tick controller or system.
			if (Viewport->IsPlayInEditorViewport())
			{
				return false;
			}
		}
	}
#endif
	if (World->IsGameWorld() && !World->HasBegunPlay())
	{
		return false;
	}
	if (World->IsGameWorld())
	{
		if (GlobalController && !GlobalController->bSimulateInGame)
		{
			return false;
		}
	}
#if WITH_EDITOR
	else
	{
		if (GlobalController && !GlobalController->bSimulateInEditor)
		{
			return false;
		}
	}
#endif
	return true;
}

void UAetherWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SCOPE_CYCLE_COUNTER(STAT_AetherWorldSubsystem_Tick);
	
	EvaluateActiveControllers();
	UpdateSourceCoordinate();
	UpdateSystemState_DielRhythm(DeltaTime);
	UpdateSystemStateFromActiveControllers(DeltaTime);
	UpdateWorld();
	
#if UE_ENABLE_DEBUG_DRAWING
	if (CVarVisualizeAetherState.GetValueOnGameThread() > 0 && GEngine)
	{
		FString DebugString = FString("Aether System State:\n") + SystemState.ToString();
		GEngine->AddOnScreenDebugMessage((uint64)this + 0, 1.0f, FColor::Green, DebugString);
	}
	
	if (CVarVisualizeAetherControllerWeight.GetValueOnGameThread() > 0 && GEngine)
	{
		FString DebugString = FString("Aether Controllers weight map:\n");
		for (auto It = ActiveControllers.CreateConstIterator(); It; ++It)
		{
			DebugString += It.Key()->GetActorLabel() + FString(": ") + FString::SanitizeFloat(It.Value()) + FString("\n");
		}
		GEngine->AddOnScreenDebugMessage((uint64)this + 1, 1.0f, FColor::Green, DebugString);
	}
	
	if (CVarVisualizeAetherControllerPoint.GetValueOnGameThread() > 0 && GEngine)
	{
		for (auto It = ActiveControllers.CreateConstIterator(); It; ++It)
		{
			It.Key()->DrawDebugPointInfo(FMath::Lerp(FLinearColor::Red, FLinearColor::Green, It.Value()).ToFColor(true));
		}
	}
#endif
}

void UAetherWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	// This is called earlier than any Actor's BeginPlay that does register work.
	//InitializeAetherSystem();
	InWorld.OnWorldBeginPlay.AddUObject(this, &UAetherWorldSubsystem::PostWorldBeginPlay);
}

UAetherWorldSubsystem* UAetherWorldSubsystem::Get(UObject* ContextObject)
{
	UAetherWorldSubsystem* Subsystem = Cast<UAetherWorldSubsystem>(USubsystemBlueprintLibrary::GetWorldSubsystem(ContextObject, StaticClass()));
	return Subsystem;
}

void UAetherWorldSubsystem::RegisterController(AAetherControllerBase* InController)
{
	if (!InController)
	{
		return;
	}
	if (AAetherGlobalController* LocalGlobalController = Cast<AAetherGlobalController>(InController))
	{
		GlobalController = LocalGlobalController;
	}
	else if (AAetherAreaController* AreaController = Cast<AAetherAreaController>(InController))
	{
		AreaControllers.AddUnique(AreaController);
	}
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			// Refresh for Editor World.
			InitializeAetherSystem();
		}
	}
#endif
}

void UAetherWorldSubsystem::UnregisterController(AAetherControllerBase* InController)
{
	if (!InController)
	{
		return;
	}
	if (GlobalController == InController)
	{
		GlobalController = nullptr;
	}
	else if (AAetherAreaController* AreaController = Cast<AAetherAreaController>(InController))
	{
		AreaControllers.Remove(AreaController);
		ActiveControllers.Remove(AreaController);
	}
}

void UAetherWorldSubsystem::RegisterAvatar(AAetherAvatarBase* InAvatar)
{
	if (!InAvatar)
	{
		return;
	}
	if (AAetherLightingAvatar* InLightingAvatar = Cast<AAetherLightingAvatar>(InAvatar))
	{
		LightingAvatar = InLightingAvatar;
	}
	else if (AAetherCloudAvatar* InCloudAvatar = Cast<AAetherCloudAvatar>(InAvatar))
	{
		CloudAvatar = InCloudAvatar;
	}
	Avatars.AddUnique(InAvatar);
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			// Refresh for Editor World.
			InitializeAetherSystem();
		}
	}
#endif
}

void UAetherWorldSubsystem::UnregisterAvatar(AAetherAvatarBase* InAvatar)
{
	if (!InAvatar)
	{
		return;
	}
	if (LightingAvatar == InAvatar)
	{
		LightingAvatar = nullptr;
	}
	else if (CloudAvatar == InAvatar)
	{
		CloudAvatar = nullptr;
	}
	Avatars.Remove(InAvatar);
}

void UAetherWorldSubsystem::TriggerWeatherEventImmediately(const FGameplayTag& EventTag)
{
	
}

void UAetherWorldSubsystem::InitializeAetherSystem()
{
	SystemState.Reset();
	StreamingSourceLocation = FVector4f::Zero();
	StreamingSourceLocation.W = -1.0f;
	EvaluateActiveControllers();
	UpdateSourceCoordinate();
	if (GlobalController)
	{
		SystemState.ProgressOfYear = FMath::Frac(GlobalController->InitTimeStampOfYear / (GlobalController->PeriodOfDay * GlobalController->DaysOfMonth * 12));
	}
	UpdateSystemState_DielRhythm(0.0f);
	UpdateSystemStateFromActiveControllers(0.0f);
	UpdateWorld();
}

void UAetherWorldSubsystem::PostWorldBeginPlay()
{
	// Game Word initialize once. All the actors have registered.
	InitializeAetherSystem();
}

#if WITH_EDITOR
void UAetherWorldSubsystem::OnMapOpened(const FString& Filename, bool bAsTemplate)
{
	if (!bAsTemplate)
	{
		// Game Word initialize once. All the actors have registered.
		InitializeAetherSystem();
	}
}
#endif

void UAetherWorldSubsystem::EvaluateActiveControllers()
{
	ActiveControllers.Reset();
	
	StreamingSourceLocation.W = -1.0f;
	if (UWorld* World = GetWorld())
	{
#if WITH_EDITOR
		if (GEditor && World->WorldType == EWorldType::Type::Editor)
		{
			if (const FViewport* Viewport = GEditor->GetActiveViewport())
			{
				if (FViewportClient* ViewportClient = Viewport->GetClient())
				{
					if (const FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(ViewportClient))
					{
						FVector ViewLocation = EditorViewportClient->GetViewLocation();
						StreamingSourceLocation.Set(ViewLocation.X, ViewLocation.Y, ViewLocation.Z, 1.0f);
					}
				}
			}
		}
		else
		{
#endif
			check(World->IsGameWorld());
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				if (const AActor* ViewTarget = PlayerController->GetViewTarget())
				{
					FVector ViewTargetLocation = ViewTarget->GetActorLocation();
					StreamingSourceLocation.Set(ViewTargetLocation.X, ViewTargetLocation.Y, ViewTargetLocation.Z, 1.0f);
				}
				else
				{
					FVector ViewPointLocation;
					FRotator Rotation;
					PlayerController->GetPlayerViewPoint(ViewPointLocation, Rotation);
					StreamingSourceLocation.Set(ViewPointLocation.X, ViewPointLocation.Y, ViewPointLocation.Z, 1.0f);
				}
			}
#if WITH_EDITOR
		}
#endif
	}
	
	if (StreamingSourceLocation.W > 0.0f)
	{
		float WeightSum = 0.0f;
		TMap<AAetherAreaController*, float> ControllerDistanceMap;
		for (AAetherAreaController* Controller : AreaControllers)
		{
			if (Controller)
			{
				float Dis = FVector::Distance(Controller->GetActorLocation(), FVector(StreamingSourceLocation));
				Dis = FMath::Max(Dis - Controller->GetAffectRadius(), UE_SMALL_NUMBER);
				float Weight = 1.0f / FMath::Pow(Dis, 2.0f);
				ControllerDistanceMap.Add(Controller, Weight);
				WeightSum += Weight;
			}
		}
		for (auto It = ControllerDistanceMap.CreateConstIterator(); It; ++It)
		{
			float NormalizedWeight = It.Value() / WeightSum;
			if (NormalizedWeight > UE_KINDA_SMALL_NUMBER)
			{
				ActiveControllers.Add(It.Key(), NormalizedWeight);
			}
		}
	}
}

void UAetherWorldSubsystem::UpdateSourceCoordinate()
{
	if (GlobalController)
	{
		if (StreamingSourceLocation.W > 0.0f)
		{
			FVector Offset = FVector(StreamingSourceLocation) - GlobalController->GetActorLocation();
            Offset.Z = 0.0f;
            FVector NorthDirection = FRotator(0.0f, GlobalController->NorthDirectionYawOffset, 0.0f).Vector();
            FVector EastDirection = FRotator(0.0f, GlobalController->NorthDirectionYawOffset + 90.0f, 0.0f).Vector();
            float NorthDis = FVector::DotProduct(Offset, NorthDirection);
            float EastDis = FVector::DotProduct(Offset, EastDirection);
            SystemState.Latitude = GlobalController->Latitude + NorthDis / GlobalController->NorthDisPerDegreeLatitude / 100.0f;
            SystemState.Longitude = GlobalController->Longitude + EastDis / GlobalController->EastDisPerDegreeLongitude / 100.0;
		}
		else
		{
			SystemState.Latitude = GlobalController->Latitude;
			SystemState.Longitude = GlobalController->Longitude;
		}
	}
}

void UAetherWorldSubsystem::UpdateSystemState_DielRhythm(float DeltaTime)
{
	if (1)
	{
		UpdateSystemState_DielRhythm_Earth(DeltaTime);
	}
	else
	{
		UpdateSystemState_DielRhythm_Custom(DeltaTime);
	}
}

void UAetherWorldSubsystem::UpdateSystemState_DielRhythm_Earth(float DeltaTime)
{
	if (GlobalController)
	{
		float DaytimeSpeedScale = ActiveControllers.Num() > 0 ? 0.0f : 1.0f;
        float NightSpeedScale = ActiveControllers.Num() > 0 ? 0.0f : 1.0f;
        for (auto It = ActiveControllers.CreateConstIterator(); It; ++It)
        {
        	DaytimeSpeedScale += It.Key()->DaytimeSpeedScale * It.Value();
        	NightSpeedScale += It.Key()->NightSpeedScale * It.Value();
        }
        float DielDeltaTime = DeltaTime * (SystemState.SunLightDirection.Z < 0.0f ? DaytimeSpeedScale : NightSpeedScale);
        SystemState.ProgressOfYear += DielDeltaTime / (GlobalController->PeriodOfDay * GlobalController->DaysOfMonth * 12);
        SystemState.ProgressOfYear = FMath::Frac(SystemState.ProgressOfYear);
        // Todo
        SystemState.Time += DeltaTime;
	}
	UpdatePlanetByTime();
}

void UAetherWorldSubsystem::UpdatePlanetByTime()
{
	if (GlobalController)
	{
		int32 PolarCondition = 0;
        float SunElevation = 0.0f;
        float SunAzimuth = 0.0f;
        const float ProgressOfDay = FMath::Frac(SystemState.ProgressOfYear * GlobalController->DaysOfMonth * 12);
		const float TimeStampOfEarthDay = ProgressOfDay * SECONDS_PER_DAY_EARTH;
		const float TimeStampOfEarthYear = SystemState.ProgressOfYear * SECONDS_PER_YEAR_EARTH;
        CalculateSunPosition(
        	SystemState.Latitude,
        	SystemState.Longitude,
        	TimeStampOfEarthDay,
        	TimeStampOfEarthYear,
        	SunElevation,
        	SunAzimuth,
        	PolarCondition);
        SystemState.SunLightDirection = ConvertPlanetLightDirection(SunElevation, SunAzimuth);
		
		float MoonElevation = 0.0f;
		float MoonAzimuth = 0.0f;
		CalculateMoonPosition(
			SystemState.Latitude,
			SystemState.Longitude,
			TimeStampOfEarthDay,
			TimeStampOfEarthYear,
			MoonElevation,
			MoonAzimuth);
		SystemState.MoonLightDirection = ConvertPlanetLightDirection(MoonElevation, MoonAzimuth);
	}
}

void UAetherWorldSubsystem::UpdateSystemState_DielRhythm_Custom(float DeltaTime)
{
	
}

void UAetherWorldSubsystem::UpdateSystemStateFromActiveControllers(float DeltaTime)
{
	EvaluateWeatherEvent(DeltaTime);
	UpdateWeatherEvent(DeltaTime);
	// Todo
	//CalcSurfaceCoeffcient(ActualDeltaTime);
}

void UAetherWorldSubsystem::EvaluateWeatherEvent(float DeltaTime)
{
	if (ActiveControllers.Num() > 0)
	{
		
	}
}

void UAetherWorldSubsystem::UpdateWeatherEvent(float DeltaTime)
{
	
}

void UAetherWorldSubsystem::UpdateWorld()
{
	UpdateAvatar();
	UpdateSystemMaterialParameter();
}

void UAetherWorldSubsystem::UpdateAvatar()
{
	for (AAetherAvatarBase* Avatar : Avatars)
	{
		if (Avatar)
		{
			Avatar->UpdateFromSystemState(SystemState);
		}
	}
}

void UAetherWorldSubsystem::UpdateSystemMaterialParameter()
{
	if (!SystemMaterialParameterCollection)
	{
		return;
	}
	{
		FLinearColor ExistValue = UKismetMaterialLibrary::GetVectorParameterValue(this, SystemMaterialParameterCollection, FName("SunLightDirection"));
		if (!SystemState.SunLightDirection.Equals(FVector(ExistValue.R, ExistValue.G, ExistValue.B)))
		{
			UKismetMaterialLibrary::SetVectorParameterValue(this, SystemMaterialParameterCollection, FName("SunLightDirection"), FLinearColor(SystemState.SunLightDirection.X, SystemState.SunLightDirection.Y, SystemState.SunLightDirection.Z, 0.0f));
		}
	}
	{
		FLinearColor ExistValue = UKismetMaterialLibrary::GetVectorParameterValue(this, SystemMaterialParameterCollection, FName("MoonLightDirection"));
		if (!SystemState.MoonLightDirection.Equals(FVector(ExistValue.R, ExistValue.G, ExistValue.B)))
		{
			UKismetMaterialLibrary::SetVectorParameterValue(this, SystemMaterialParameterCollection, FName("MoonLightDirection"), FLinearColor(SystemState.MoonLightDirection.X, SystemState.MoonLightDirection.Y, SystemState.MoonLightDirection.Z, 0.0f));
		}
	}
	{
		FLinearColor ExistValue = UKismetMaterialLibrary::GetVectorParameterValue(this, SystemMaterialParameterCollection, FName("WindData"));
		if (!SystemState.WindData.Equals(FVector4f(ExistValue.R, ExistValue.G, ExistValue.B, ExistValue.A)))
		{
			UKismetMaterialLibrary::SetVectorParameterValue(this, SystemMaterialParameterCollection, FName("WindData"), FLinearColor(SystemState.WindData.X, SystemState.WindData.Y, SystemState.WindData.Z, SystemState.WindData.W));
		}
	}
	
	{
		float ExistValue = UKismetMaterialLibrary::GetScalarParameterValue(this, SystemMaterialParameterCollection, FName("SurfaceRainRemain"));
		if (!FMath::IsNearlyEqual(SystemState.SurfaceRainRemain, ExistValue, UE_KINDA_SMALL_NUMBER))
		{
			UKismetMaterialLibrary::SetScalarParameterValue(this, SystemMaterialParameterCollection, FName("SurfaceRainRemain"), SystemState.SurfaceRainRemain);
		}
	}
	{
		float ExistValue = UKismetMaterialLibrary::GetScalarParameterValue(this, SystemMaterialParameterCollection, FName("SurfaceSnowDepth"));
		if (!FMath::IsNearlyEqual(SystemState.SurfaceSnowDepth, ExistValue, UE_KINDA_SMALL_NUMBER))
		{
			UKismetMaterialLibrary::SetScalarParameterValue(this, SystemMaterialParameterCollection, FName("SurfaceSnowDepth"), SystemState.SurfaceSnowDepth);
		}
	}
	{
		float ExistValue = UKismetMaterialLibrary::GetScalarParameterValue(this, SystemMaterialParameterCollection, FName("ProgressOfYear"));
		if (!FMath::IsNearlyEqual(SystemState.ProgressOfYear, ExistValue, UE_KINDA_SMALL_NUMBER))
		{
			UKismetMaterialLibrary::SetScalarParameterValue(this, SystemMaterialParameterCollection, FName("ProgressOfYear"), SystemState.ProgressOfYear);
		}
	}
}