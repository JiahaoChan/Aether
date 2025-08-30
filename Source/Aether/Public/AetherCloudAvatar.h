/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AetherTypes.h"

#include "AetherCloudAvatar.generated.h"

UCLASS()
class AETHER_API AAetherCloudAvatar : public AActor
{
	GENERATED_BODY()
	
private:
	
public:
	AAetherCloudAvatar();
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void Destroyed() override;
	
public:
	void UpdateCloudLayers(const FAetherState& State);
};