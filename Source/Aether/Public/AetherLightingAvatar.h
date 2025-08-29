/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AetherTypes.h"

#include "AetherLightingAvatar.generated.h"

UCLASS()
class AETHER_API AAetherLightingAvatar : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(Category = "Aether|Component", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SkyDomeComponent;
	
	UPROPERTY(Category = "Aether|Component", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDirectionalLightComponent> SunLightComponent;
	
	UPROPERTY(Category = "Aether|Component", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDirectionalLightComponent> MoonLightComponent;
	
	UPROPERTY(Category = "Aether|Component", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkyLightComponent> SkyLightComponent;
	
public:
	AAetherLightingAvatar();
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
public:
	void UpdateLighting(const FAetherState& State);
};