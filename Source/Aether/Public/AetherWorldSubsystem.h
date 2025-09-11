/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "AetherTypes.h"

#include "AetherWorldSubsystem.generated.h"

UCLASS(NotBlueprintable)
class AETHER_API UAetherWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	TObjectPtr<class AAetherGlobalController> GlobalController;
	
	UPROPERTY()
	TArray<TObjectPtr<class AAetherAreaController>> AreaControllers;
	
	UPROPERTY()
	TMap<TObjectPtr<AAetherAreaController>, float> ActiveControllers;
	
	UPROPERTY()
	TArray<TObjectPtr<class AAetherAvatarBase>> Avatars;
	
	UPROPERTY()
	TObjectPtr<class AAetherLightingAvatar> LightingAvatar;
	
	UPROPERTY()
	TObjectPtr<class AAetherCloudAvatar> CloudAvatar;
	
	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> SystemMaterialParameterCollection;
	
	UPROPERTY()
	FAetherState SystemState;
	
	// Cache for calculation.
	FVector4f StreamingSourceLocation;
	
public:
	UAetherWorldSubsystem();
	
	//~ Begin UTickableWorldSubsystem Interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UAetherWorldSubsystem, STATGROUP_Tickables); }
	
	virtual bool IsTickable() const override;
	
	virtual void Tick(float DeltaTime) override;
	
	virtual bool IsTickableInEditor() const override { return true; }
	
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	//~ End UTickableWorldSubsystem Interface
	
public:
	//~ Begin UAetherWorldSubsystem Interface
	static UAetherWorldSubsystem* Get(UObject* ContextObject);
	
	void RegisterController(class AAetherControllerBase* InController);
	void UnregisterController(AAetherControllerBase* InController);
	
	void RegisterAvatar(AAetherAvatarBase* InAvatar);
	void UnregisterAvatar(AAetherAvatarBase* InAvatar);
	
	void TriggerWeatherEventImmediately(const FGameplayTag& EventTag);
	
	void InitializeAetherSystem();
	//~ End UAetherWorldSubsystem Interface
	
protected:
	void PostWorldBeginPlay();
	
#if WITH_EDITOR
	void OnMapOpened(const FString& Filename, bool bAsTemplate);
#endif
	
	void EvaluateActiveControllers();
	
	void UpdateSourceCoordinate();
	
	void UpdateSystemState_DielRhythm(float DeltaTime);
	
	void UpdateSystemState_DielRhythm_Earth(float DeltaTime);
	void UpdatePlanetByTime();
	
	void UpdateSystemState_DielRhythm_Custom(float DeltaTime);
	
	void UpdateSystemStateFromActiveControllers(float DeltaTime);
	
	void EvaluateWeatherEvent(float DeltaTime);
	void UpdateWeatherEvent(float DeltaTime);
	
	void UpdateWorld();
	
	void UpdateAvatar();
	
	void UpdateSystemMaterialParameter();
	
public:
	FORCEINLINE const TMap<TObjectPtr<AAetherAreaController>, float>& GetActiveControllers() const { return ActiveControllers; }
	FORCEINLINE const FAetherState& GetSystemState() const { return SystemState; }
};