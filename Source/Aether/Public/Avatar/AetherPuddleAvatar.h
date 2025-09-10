/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"

#include "Avatar/AetherAvatarBase.h"

#include "AetherPuddleAvatar.generated.h"

UCLASS(Abstract)
class AETHER_API AAetherPuddleAvatarBase : public AAetherAvatarBase
{
	GENERATED_BODY()
	
public:
	AAetherPuddleAvatarBase();
	
protected:
	virtual void BeginPlay() override;
	
public:
	//~ Begin Aether Interface
	virtual bool IsGlobalAvatar() const override { return false; }
	//~ End Aether Interface
};

UCLASS()
class AETHER_API AAetherPuddleAvatar_Plane : public AAetherPuddleAvatarBase
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WaterSurfaceMeshComponent;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether|Puddle")
	float ConstantHeight;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether|Puddle")
	float MaxHeight;
	
public:
	AAetherPuddleAvatar_Plane();
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	//~ Begin Aether Interface
	virtual void UpdateFromSystemState(const FAetherState& State) override;
	//~ End Aether Interface
};

UCLASS()
class AETHER_API AAetherPuddleAvatar_Decal : public AAetherPuddleAvatarBase
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDecalComponent> WaterSurfaceDecalComponent;
	
protected:
	
public:
	AAetherPuddleAvatar_Decal();
	
	//~ Begin Aether Interface
	//virtual void UpdateFromSystemState(const FAetherState& State) override;
	//~ End Aether Interface
};