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
#include "AetherLightingAvatar.h"
#include "AetherPluginSettings.h"
#include "AetherStats.h"
#include "Rendering/AetherSceneViewExtension.h"

UAetherWorldSubsystem::UAetherWorldSubsystem()
{
	
}

bool UAetherWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> DerivedClassList;
	GetDerivedClasses(GetClass(), DerivedClassList, false);
	return DerivedClassList.Num() == 0;
}

void UAetherWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	const UAetherPluginSettings* Settings = GetDefault<UAetherPluginSettings>();
	
	if (UMaterialParameterCollection* ParameterCollection = Settings->SystemMaterialParameterCollection.LoadSynchronous())
	{
		SystemMaterialParameterCollection = ParameterCollection;
	}
	else
	{
		// Log
	}
	
	Controllers.Empty();
	ActiveControllers.Empty();
	LightingAvatar = nullptr;
	
	Super::Initialize(Collection);
}

UAetherWorldSubsystem* UAetherWorldSubsystem::Get(UObject* ContextObject)
{
	UAetherWorldSubsystem* Subsystem = Cast<UAetherWorldSubsystem>(USubsystemBlueprintLibrary::GetWorldSubsystem(ContextObject, StaticClass()));
	return Subsystem;
}

bool UAetherWorldSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return Super::DoesSupportWorldType(WorldType);
}

bool UAetherWorldSubsystem::IsTickable() const
{
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::EditorPreview || World->WorldType == EWorldType::Type::GameRPC || World->WorldType == EWorldType::Type::Inactive || World->WorldType == EWorldType::Type::None)
		{
			return false;
		}
#if WITH_EDITOR
		if (World->WorldType == EWorldType::Type::Editor && GEditor)
		{
			if (const FViewport* Viewport = GEditor->GetActiveViewport())
			{
				// Editor world still maintains when PIE, but it can not be ever tick controller or system.
				if (Viewport->IsPlayInEditorViewport())
				{
					return false;
				}
			}
		}
#endif
		return true;
	}
	return false;
}

void UAetherWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SCOPE_CYCLE_COUNTER(STAT_AetherWorldSubsystem_Tick);
	
	EvaluateAndTickActiveControllers(DeltaTime);
	
	UpdateSystemState();
	
	UpdateWorld();
}

void UAetherWorldSubsystem::RegisterController(AAetherAreaController* InController)
{
	if (!InController)
	{
		return;
	}
	Controllers.AddUnique(InController);
}

void UAetherWorldSubsystem::UnRegisterController(AAetherAreaController* InController)
{
	if (!InController)
	{
		return;
	}
	Controllers.Remove(InController);
	ActiveControllers.Remove(InController);
}

void UAetherWorldSubsystem::RegisterLightingAvatar(AAetherLightingAvatar* InAvatar)
{
	if (!InAvatar)
	{
		return;
	}
	LightingAvatar = InAvatar;
}

void UAetherWorldSubsystem::UnregisterLightingAvatar(AAetherLightingAvatar* InAvatar)
{
	if (!InAvatar)
	{
		return;
	}
	if (LightingAvatar == InAvatar)
	{
		LightingAvatar = nullptr;
	}
}

void UAetherWorldSubsystem::RegisterCloudAvatar(AAetherCloudAvatar* InAvatar)
{
	if (!InAvatar)
	{
		return;
	}
	CloudAvatar = InAvatar;
}

void UAetherWorldSubsystem::UnregisterCloudAvatar(AAetherCloudAvatar* InAvatar)
{
	if (!InAvatar)
	{
		return;
	}
	if (CloudAvatar == InAvatar)
	{
		CloudAvatar = nullptr;
	}
}

void UAetherWorldSubsystem::TriggerWeatherEventImmediately(const FGameplayTag& EventTag)
{
	
}

#if WITH_EDITOR
void UAetherWorldSubsystem::ModifyAllControllersSimulationPlanetType_Editor(const ESimulationPlanetType& NewValue)
{
	for (AAetherAreaController* Controller : Controllers)
	{
		if (Controller)
		{
			if (Controller->GetSimulationPlanet() != NewValue)
			{
				Controller->SetSimulationPlanet(NewValue);
				Controller->MarkPackageDirty();
			}
		}
	}
}

void UAetherWorldSubsystem::SyncOtherControllerDielRhythm_Editor(const AAetherAreaController* CenterController)
{
	if (!CenterController)
	{
		return;
	}
	for (AAetherAreaController* OtherController : Controllers)
	{
		if (OtherController)
		{
			bool bDirty = false;
			if (OtherController->PeriodOfDay != CenterController->PeriodOfDay)
			{
				OtherController->PeriodOfDay = CenterController->PeriodOfDay;
				bDirty |= true;
			}
			if (OtherController->DaysOfMonth != CenterController->DaysOfMonth)
			{
				OtherController->DaysOfMonth = CenterController->DaysOfMonth;
				bDirty |= true;
			}
			if (OtherController->DaytimeSpeedScale != CenterController->DaytimeSpeedScale)
			{
				OtherController->DaytimeSpeedScale = CenterController->DaytimeSpeedScale;
				bDirty |= true;
			}
			if (OtherController->NightSpeedScale != CenterController->NightSpeedScale)
			{
				OtherController->NightSpeedScale = CenterController->NightSpeedScale;
				bDirty |= true;
			}
			if (OtherController->NorthDirectionYawOffset != CenterController->NorthDirectionYawOffset)
			{
				OtherController->NorthDirectionYawOffset = CenterController->NorthDirectionYawOffset;
				bDirty |= true;
			}
			if (bDirty)
			{
				OtherController->MarkPackageDirty();
			}
		}
	}
}

void UAetherWorldSubsystem::CorrectOtherControllerInitTimeStamp_Editor(const AAetherAreaController* CenterController)
{
	if (!CenterController)
	{
		return;
	}
	const float& Longitude = CenterController->Longitude;
	for (AAetherAreaController* OtherController : Controllers)
	{
		if (OtherController)
		{
			
		}
	}
}
#endif

void UAetherWorldSubsystem::EvaluateAndTickActiveControllers(float DeltaTime)
{
	bool bCouldTickController = false;
	FVector StreamingSourceLocation = FVector::ZeroVector;
#if WITH_EDITOR
	if (UWorld* World = GetWorld())
	{
		if (GEditor && World->WorldType == EWorldType::Type::Editor)
		{
			if (const FViewport* Viewport = GEditor->GetActiveViewport())
			{
				if (FViewportClient* ViewportClient = Viewport->GetClient())
				{
					if (const FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(ViewportClient))
					{
						StreamingSourceLocation = EditorViewportClient->GetViewLocation();
						bCouldTickController = true;
					}
				}
			}
		}
		else
		{
			check(World->IsGameWorld());
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				FRotator Rotation;
				PlayerController->GetPlayerViewPoint(StreamingSourceLocation, Rotation);
				bCouldTickController = true;
			}
		}
	}
#else
	if (UWorld* World = GetWorld())
	{
		check(World->IsGameWorld());
        if (APlayerController* PlayerController = World->GetFirstPlayerController())
        {
        	FRotator Rotation;
        	PlayerController->GetPlayerViewPoint(StreamingSourceLocation, Rotation);
        	bCouldTickController = true;
        }
	}
#endif
	
	ActiveControllers.Reset();
	if (bCouldTickController)
	{
		TArray<TObjectPtr<AAetherAreaController>> SortedList = Controllers;
		if (SortedList.Num() > 1)
		{
			SortedList.Sort([StreamingSourceLocation](const TObjectPtr<AAetherAreaController>& A, const TObjectPtr<AAetherAreaController>& B) {
            	return FVector::Distance(A->GetActorLocation(), StreamingSourceLocation) < FVector::Distance(B->GetActorLocation(), StreamingSourceLocation);
            });
		}
		for (int32 i = 0; i < 2 && i < SortedList.Num(); i++)
		{
			ActiveControllers.Add(SortedList[i], 1.0f);
		}
	}
	
	for (AAetherAreaController* Controller : Controllers)
	{
		check(Controller);
		if (ActiveControllers.Contains(Controller))
		{
			Controller->TickAetherController(DeltaTime);
		}
		else
		{
			Controller->IncSinceLastTickTime(DeltaTime);
		}
	}
}

void UAetherWorldSubsystem::UpdateSystemState()
{
	SystemState.Reset();
	for (auto It = ActiveControllers.CreateConstIterator(); It; ++It)
	{
		SystemState = SystemState + It.Key()->GetCurrentState() * It.Value();
	}
	SystemState.Normalize();
}

void UAetherWorldSubsystem::UpdateWorld()
{
	UpdateAvatar();
	UpdateSystemMaterialParameter();
}

void UAetherWorldSubsystem::UpdateAvatar()
{
	if (LightingAvatar)
	{
		LightingAvatar->UpdateLighting(SystemState);
	}
	if (CloudAvatar)
	{
		CloudAvatar->UpdateCloudLayers(SystemState);
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