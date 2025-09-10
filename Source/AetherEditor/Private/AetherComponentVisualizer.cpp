/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherComponentVisualizer.h"

#include "AetherAreaController.h"
#include "AetherSettingsInfo.h"
#include "AetherWorldSubsystem.h"

void FAetherSettingsComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (const UAetherSettingsVisualizeComponent* VisualizeComponent = Cast<UAetherSettingsVisualizeComponent>(Component))
	{
		if (const AActor* Owner = VisualizeComponent->GetOwner())
		{
			const FMatrix LocalToWorld = Owner->GetActorTransform().ToMatrixWithScale();
			FLinearColor DrawColor = FColor(0, 157, 0, 255);
			DrawOrientedWireBox(PDI,
				LocalToWorld.GetOrigin(),
				LocalToWorld.GetScaledAxis(EAxis::X),
				LocalToWorld.GetScaledAxis(EAxis::Y),
				LocalToWorld.GetScaledAxis(EAxis::Z),
				FVector(100.0f),
				DrawColor,
				SDPG_World);
		}
	}
}

void FAetherContollerComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (const UAetherControllerVisualizeComponent* VisualizeComponent = Cast<UAetherControllerVisualizeComponent>(Component))
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
			DrawWireCylinder(PDI,
				LocalToWorld.GetOrigin(),
				LocalToWorld.GetScaledAxis(EAxis::X),
				LocalToWorld.GetScaledAxis(EAxis::Y),
				LocalToWorld.GetScaledAxis(EAxis::Z),
				DrawColor,
				Owner->GetAffectRadius(),
				FMath::Max(Owner->GetAffectRadius() * 3.0f, 1000.0f) / 2.0f,
				16,
				SDPG_World);
		}
	}
}