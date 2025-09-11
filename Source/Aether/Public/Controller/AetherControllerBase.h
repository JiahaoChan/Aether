/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AetherControllerBase.generated.h"

UCLASS(HideCategories = ("Collision", "Physics", "Cooking", "Navigation", "Networking", "Replication", "Input", "Rendering", "HLOD"))
class AETHER_API AAetherControllerBase : public AActor
{
	GENERATED_BODY()
	
	friend class UAetherWorldSubsystem;
	
public:
	AAetherControllerBase();
	
	//~ Begin AActor Interface
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void PostInitializeComponents() override;
	
	virtual void Destroyed() override;
	
#if WITH_EDITOR
	virtual bool CanChangeIsSpatiallyLoadedFlag() const override { return false; }
#endif
	//~ End AActor Interface
	
protected:
	virtual void InitializeController() {}
};