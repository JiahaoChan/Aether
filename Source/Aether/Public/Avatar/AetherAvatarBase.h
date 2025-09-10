/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AetherTypes.h"

#include "AetherAvatarBase.generated.h"

UCLASS(Abstract)
class AETHER_API AAetherAvatarBase : public AActor
{
	GENERATED_BODY()
	
public:
	AAetherAvatarBase();
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void Destroyed() override;
	
	//~ Begin Aether Interface
	virtual bool IsGlobalAvatar() const { return true; }
	
	virtual void UpdateFromSystemState(const FAetherState& State) {}
	//~ End Aether Interface
};