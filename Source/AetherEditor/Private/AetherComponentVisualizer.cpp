/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherComponentVisualizer.h"

#include "CanvasTypes.h"
#include "Engine/Font.h"

#include "AetherAreaController.h"
#include "AetherGlobalController.h"
#include "AetherWorldSubsystem.h"

IMPLEMENT_HIT_PROXY(HPlanetSphereProxy, HComponentVisProxy)

void FAetherGlobalControllerComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (const UAetherGlobalControllerVisualizeComponent* VisualizeComponent = Cast<UAetherGlobalControllerVisualizeComponent>(Component))
	{
		if (AAetherGlobalController* Owner = VisualizeComponent->GetOwner<AAetherGlobalController>())
		{
			const FMatrix LocalToWorld = Owner->GetActorTransform().ToMatrixWithScale();
			{
				PDI->SetHitProxy(nullptr);
                FLinearColor DrawColor = FColor(0, 157, 0, 255);
                DrawOrientedWireBox(PDI,
                	LocalToWorld.GetOrigin() + FVector(0.0f, 0.0f, 150.0f),
                	LocalToWorld.GetScaledAxis(EAxis::X),
                	LocalToWorld.GetScaledAxis(EAxis::Y),
                	LocalToWorld.GetScaledAxis(EAxis::Z),
                	FVector(150.0f),
                	DrawColor,
                	SDPG_World);
			}
			
			FVector EarthLocation = LocalToWorld.GetOrigin() + FVector(0.0f, 0.0f, 150.0f);
			// Earth
			{
				PDI->SetHitProxy(nullptr);
				DrawWireSphere(PDI,
					EarthLocation,
					FLinearColor(0.159f, 0.242f, 0.913f),
					20.0f,
					32,
					SDPG_World,
					1.0f);
			}
			if (const UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(Owner))
			{
				// Sun
				{
					DrawWireSphere(PDI,
                        EarthLocation + Subsystem->GetSystemState().SunLightDirection * -150.0f,
                        FLinearColor(1.0f, 0.049f, 0.0f),
                        30.0f,
                        32,
                        SDPG_World,
                        1.0f);
                    
                    FTransform ArrowTransform = FTransform::Identity;
                    ArrowTransform.SetLocation(EarthLocation);
                    ArrowTransform.SetRotation((-Subsystem->GetSystemState().SunLightDirection).ToOrientationQuat());
                    DrawDirectionalArrow(PDI,
                        ArrowTransform.ToMatrixNoScale(),
                        FLinearColor::Green,
                        150.0f,
                        8.0f,
                        SDPG_World,
                        1.0f);
				}
				// Moon
				{
					DrawWireSphere(PDI,
                        EarthLocation + Subsystem->GetSystemState().MoonLightDirection * -100.0f,
                        FLinearColor(0.437f, 0.565f, 0.531f),
                        15.0f,
                        32,
                        SDPG_World,
                        1.0f);
                    
                    FTransform ArrowTransform = FTransform::Identity;
                    ArrowTransform.SetLocation(EarthLocation);
                    ArrowTransform.SetRotation((-Subsystem->GetSystemState().MoonLightDirection).ToOrientationQuat());
                    DrawDirectionalArrow(PDI,
                        ArrowTransform.ToMatrixNoScale(),
                        FLinearColor::Green,
                        100.0f,
                        8.0f,
                        SDPG_World,
                        1.0f);
				}
			}
		}
	}
}

void FAetherGlobalControllerComponentVisualizer::DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	if (const UAetherGlobalControllerVisualizeComponent* VisualizeComponent = Cast<UAetherGlobalControllerVisualizeComponent>(Component))
	{
		if (AAetherGlobalController* Owner = VisualizeComponent->GetOwner<AAetherGlobalController>())
		{
			const FMatrix LocalToWorld = Owner->GetActorTransform().ToMatrixWithScale();
			FVector EarthLocation = LocalToWorld.GetOrigin() + FVector(0.0f, 0.0f, 150.0f);
			FVector2D ScreenPosition;
			View->WorldToPixel(EarthLocation, ScreenPosition);
			Canvas->DrawShadowedString(ScreenPosition.X, ScreenPosition.Y, *FString("Earth"), GEngine->GetLargeFont(), FLinearColor(0.159f, 0.242f, 0.913f));
			
			if (const UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(Owner))
			{
				FString DebugString = FString("Aether System State:\n") + Subsystem->GetSystemState().ToString();
				Canvas->DrawShadowedString(45.0f, 45.0f, *DebugString, GEngine->GetLargeFont(), FLinearColor::Green);
			}
		}
	}
}

bool FAetherGlobalControllerComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	if (VisProxy && VisProxy->Component.IsValid())
	{
		SelectedComponent = Cast<UAetherGlobalControllerVisualizeComponent>(VisProxy->Component.Get());
		return true;
	}
	return false;
}

bool FAetherGlobalControllerComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (SelectedComponent.IsValid())
	{
		if (const AAetherGlobalController* Owner = SelectedComponent->GetOwner<AAetherGlobalController>())
		{
			OutLocation = Owner->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
			return true;
		}
	}
	return false;
}

UActorComponent* FAetherGlobalControllerComponentVisualizer::GetEditedComponent() const
{
	if (SelectedComponent.IsValid())
	{
		return const_cast<UAetherGlobalControllerVisualizeComponent*>(SelectedComponent.Get());
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FAetherAreaControllerComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (const UAetherAreaControllerVisualizeComponent* VisualizeComponent = Cast<UAetherAreaControllerVisualizeComponent>(Component))
	{
		if (AAetherAreaController* Owner = VisualizeComponent->GetOwner<AAetherAreaController>())
		{
			const FMatrix LocalToWorld = Owner->GetActorTransform().ToMatrixWithScale();
			FLinearColor DrawColor = FLinearColor::Red;
			if (const UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(Owner))
			{
				for (auto It = Subsystem->GetActiveControllers().CreateConstIterator(); It; ++It)
                {
					if (It.Key() == Owner)
					{
						DrawColor = FMath::Lerp(FLinearColor::Red, FLinearColor::Green, It.Value());
						break;
					}
                }
			}
			float HalfHeight = FMath::Max(Owner->GetAffectRadius() * 3.0f, 1000.0f) / 2.0f;
			DrawWireCylinder(PDI,
				LocalToWorld.GetOrigin() + FVector(0.0f, 0.0f, HalfHeight),
				LocalToWorld.GetScaledAxis(EAxis::X),
				LocalToWorld.GetScaledAxis(EAxis::Y),
				LocalToWorld.GetScaledAxis(EAxis::Z),
				DrawColor,
				Owner->GetAffectRadius(),
				HalfHeight,
				16,
				SDPG_World);
		}
	}
}