/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherWorldSubsystem.h"

#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

#include "AetherAreaController.h"
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
	if (UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::EditorPreview || World->WorldType == EWorldType::Type::GameRPC || World->WorldType == EWorldType::Type::Inactive || World->WorldType == EWorldType::Type::None)
		{
			return false;
		}
		return true;
	}
	return false;
}

void UAetherWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SCOPE_CYCLE_COUNTER(STAT_AetherWorldSubsystem_Tick);
	
#if WITH_EDITOR
	if (UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			DrawDebugString(World, FVector(0.0f, 0.0f, 200.0f), FString("AAAA"));
		}
		//DrawDebugString(World, FVector(0.0f, 0.0f, 200.0f), FString("AAAA"));
	}
#endif
	
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

void UAetherWorldSubsystem::UnRegisterLightingAvatar(AAetherLightingAvatar* InAvatar)
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
#endif

void UAetherWorldSubsystem::EvaluateAndTickActiveControllers(float DeltaTime)
{
	FVector StreamingSourceLocation = FVector::ZeroVector;
	bool bCouldTickSystem = false;
#if WITH_EDITOR
	if (UWorld* World = GetWorld())
	{
		if (GEditor && World->WorldType == EWorldType::Type::Editor)
		{
			if (const FViewport* Viewport = GEditor->GetActiveViewport())
			{
				//Viewport->IsPlayInEditorViewport()
				if (FViewportClient* ViewportClient = Viewport->GetClient())
				{
					if (const FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(ViewportClient))
                    {
                        StreamingSourceLocation = EditorViewportClient->GetViewLocation();
                        bCouldTickSystem = true;
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
				bCouldTickSystem = true;
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
        	bCouldTickSystem = true;
        }
	}
#endif
	
	ActiveControllers.Reset();
	if (bCouldTickSystem)
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
	// Todo
	int32 i = 0;
	for (auto It = ActiveControllers.CreateConstIterator(); It; ++It)
	{
		if (i == 0)
		{
			SystemState = It.Key()->GetCurrentState() * It.Value();
		}
		else
		{
			SystemState = SystemState + It.Key()->GetCurrentState() * It.Value();
		}
		i++;
	}
	SystemState.Normalize();
}

void UAetherWorldSubsystem::UpdateWorld()
{
	UpdateAvatarLighting();
	UpdateSystemMaterialParameter();
}

void UAetherWorldSubsystem::UpdateAvatarLighting()
{
	if (LightingAvatar)
	{
		LightingAvatar->UpdateLighting(SystemState);
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