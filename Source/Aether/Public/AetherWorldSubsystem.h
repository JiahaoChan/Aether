/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "AetherTypes.h"
#include "Rendering/AetherSceneViewExtension.h"

#include "AetherWorldSubsystem.generated.h"

UCLASS(NotBlueprintable)
class AETHER_API UAetherWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	TArray<TObjectPtr<class AAetherAreaController>> Controllers;
	
	UPROPERTY()
	TMap<TObjectPtr<AAetherAreaController>, float> ActiveControllers;
	
	UPROPERTY()
	TObjectPtr<class AAetherLightingAvatar> LightingAvatar;
	
	UPROPERTY()
	TObjectPtr<class AAetherCloudAvatar> CloudAvatar;
	
	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> SystemMaterialParameterCollection;
	
	UPROPERTY()
	FAetherState SystemState;
	
	//TUniquePtr<FAetherViewParameters> CachedViewUniformShaderParameters;
	
	//TMap<int32, TUniquePtr<FAetherViewParameters>> Map;
	
public:
	UAetherWorldSubsystem();
	
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	static UAetherWorldSubsystem* Get(UObject* ContextObject);
	
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UAetherWorldSubsystem, STATGROUP_Tickables); }
	
	virtual bool IsTickable() const override;
	
	virtual void Tick(float DeltaTime) override;
	
	virtual bool IsTickableInEditor() const override { return true; }
	
public:
	void RegisterController(AAetherAreaController* InController);
	void UnRegisterController(AAetherAreaController* InController);
	
	void RegisterLightingAvatar(AAetherLightingAvatar* InAvatar);
	void UnregisterLightingAvatar(AAetherLightingAvatar* InAvatar);
	
	void RegisterCloudAvatar(AAetherCloudAvatar* InAvatar);
	void UnregisterCloudAvatar(AAetherCloudAvatar* InAvatar);
	
	void TriggerWeatherEventImmediately(const FGameplayTag& EventTag);
	
#if WITH_EDITOR
	void ModifyAllControllersSimulationPlanetType_Editor(const ESimulationPlanetType& NewValue);
	
	void SyncOtherControllerDielRhythm_Editor(const AAetherAreaController* CenterController);
	
	void CorrectOtherControllerInitTimeStamp_Editor(const AAetherAreaController* CenterController);
#endif
	
protected:
	void EvaluateAndTickActiveControllers(float DeltaTime);
	
	void UpdateSystemState();
	
	void UpdateWorld();
	
	void UpdateAvatar();
	
	void UpdateSystemMaterialParameter();
};