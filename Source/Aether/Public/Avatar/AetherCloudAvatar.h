/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"

#include "AetherAvatarBase.h"
#include "AetherTypes.h"

#include "AetherCloudAvatar.generated.h"

UCLASS()
class AETHER_API AAetherCloudAvatar : public AAetherAvatarBase
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNiagaraComponent> RainFXComponent;
	
public:
	AAetherCloudAvatar();
	
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