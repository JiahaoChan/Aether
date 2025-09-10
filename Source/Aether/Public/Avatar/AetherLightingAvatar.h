/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"

#include "AetherAvatarBase.h"
#include "AetherTypes.h"

#include "AetherLightingAvatar.generated.h"

UCLASS()
class AETHER_API AAetherLightingAvatar : public AAetherAvatarBase
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SkyDomeComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDirectionalLightComponent> SunLightComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDirectionalLightComponent> MoonLightComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkyLightComponent> SkyLightComponent;
	
public:
	AAetherLightingAvatar();
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;
	
#if WITH_EDITOR
	virtual bool CanChangeIsSpatiallyLoadedFlag() const override { return false; }
#endif
	
	//~ Begin Aether Interface
	virtual void UpdateFromSystemState(const FAetherState& State) override;
	//~ End Aether Interface
};