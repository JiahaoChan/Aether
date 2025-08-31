/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AetherTypes.h"

#include "AetherAreaController.generated.h"

UCLASS(HideCategories = ("Collision", "Physics", "Cooking", "Navigation", "Networking", "Replication", "Input", "Rendering", "HLOD"))
class AETHER_API AAetherAreaController : public AActor
{
	GENERATED_BODY()
	
	friend class UAetherWorldSubsystem;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|System")
	bool bSimulateInGame;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|System")
	bool bSimulateInEditor;
#endif
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|System")
	float AffectRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|System")
	ESimulationPlanetType SimulationPlanet;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|System", meta = (EditCondition = "SimulationPlanet == ESimulationPlanetType::Earth", EditConditionHides))
	TObjectPtr<class UAetherSystemPreset> EarthLocationPreset;
#endif
	
	// Diel Rhythm Begins
	// 纬度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Diel Rhythm", meta = (EditCondition = "SimulationPlanet == ESimulationPlanetType::Earth && ControllerRange == EAetherControllerRangeType::Global", EditConditionHides))
	float Latitude;
	
	// 经度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Diel Rhythm", meta = (EditCondition = "SimulationPlanet == ESimulationPlanetType::Earth && ControllerRange == EAetherControllerRangeType::Global", EditConditionHides))
	float Longitude;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Diel Rhythm", meta = (EditCondition = "SimulationPlanet == ESimulationPlanetType::Earth && ControllerRange == EAetherControllerRangeType::Global", EditConditionHides, ClampMin = "0.0"))
	float PeriodOfDay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Diel Rhythm", meta = (EditCondition = "SimulationPlanet == ESimulationPlanetType::Earth && ControllerRange == EAetherControllerRangeType::Global", EditConditionHides, ClampMin = "1"))
	int32 DaysOfMonth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Diel Rhythm", meta = (EditCondition = "SimulationPlanet == ESimulationPlanetType::Earth && ControllerRange == EAetherControllerRangeType::Global", EditConditionHides))
	float DaytimeSpeedScale;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Diel Rhythm", meta = (EditCondition = "SimulationPlanet == ESimulationPlanetType::Earth && ControllerRange == EAetherControllerRangeType::Global", EditConditionHides))
	float NightSpeedScale;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Diel Rhythm", meta = (EditCondition = "SimulationPlanet == ESimulationPlanetType::Earth && ControllerRange == EAetherControllerRangeType::Global", EditConditionHides))
	float NorthDirectionYawOffset;
	// Diel Rhythm Ends
	
	// Weather Begins
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Weather")
	TArray<struct FWeatherEventDescription> PossibleWeatherEvents;
	// Weather Ends
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|State")
	float InitTimeStampOfYear;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|State")
	FGameplayTagContainer InitWeatherEventTags;
	
	UPROPERTY(Transient, DuplicateTransient, VisibleAnywhere, BlueprintReadWrite, Category = "Aether|State")
	FAetherState CurrentState;
	
	UPROPERTY(Transient, DuplicateTransient)
	FAetherState LastState;
	
	UPROPERTY(Transient, DuplicateTransient)
	TArray<TObjectPtr<class UAetherWeatherEventInstance>> ActiveWeatherInstance;
	
	// Cache
	UPROPERTY(Transient, DuplicateTransient)
	FGameplayTagContainer ActiveWeatherTags;
	
	// Cache
	UPROPERTY(Transient, DuplicateTransient)
	FGameplayTagContainer BlockingWeatherTags;
	
	float SinceLastTickTime;
	
public:
	AAetherAreaController();
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	
	virtual void PostLoad() override;
	
	virtual void Destroyed() override;
	
protected:
	void Initialize();
	
	virtual void TickAetherController(float DeltaTime);
	
	void CalcControllerState_DielRhythm(float DeltaTime);
	
	virtual void CalcSystemState_DielRhythm_Earth(float DeltaTime);
	void UpdateSunByTime();
	
	virtual void CalcSystemState_DielRhythm_Custom(float DeltaTime);
	
	virtual void EvaluateWeatherEvent(float DeltaTime);
	virtual void UpdateWeatherEvent(float DeltaTime);
	
public:
    void TriggerWeatherEventImmediately(const FGameplayTag& EventTag);
    void TriggerWeatherEventImmediately(const FGameplayTagContainer& EventTags);
    void TriggerWeatherEventImmediately(const class UAetherWeatherEvent* EventClass);
    
    void CancelWeatherEventImmediately(const FGameplayTag& EventTag);
    void CancelWeatherEventImmediately(const FGameplayTagContainer& EventTags);
    void CancelWeatherEventImmediately(const UAetherWeatherEvent* EventClass);
    void CancelWeatherEventImmediately(UAetherWeatherEventInstance* EventInstance);
	
private:
	void SetWeatherInstanceState(UAetherWeatherEventInstance* InInstance, const EWeatherEventExecuteState& NewState);
	
private:
#if WITH_EDITOR
	UFUNCTION(CallInEditor, Category = "Aether")
	void CaptureTimeStamp();
	
	UFUNCTION(CallInEditor, Category = "Aether")
	void SyncOtherControllerDielRhythm();
	
	UFUNCTION(CallInEditor, Category = "Aether")
	void CorrectOtherControllerInitTimeStamp();
#endif
	
public:
	FORCEINLINE const float& GetAffectRadius() const { return AffectRadius; }
	
	FORCEINLINE const ESimulationPlanetType& GetSimulationPlanet() const { return SimulationPlanet; }
	void SetSimulationPlanet(const ESimulationPlanetType& NewValue);
	
	FORCEINLINE FAetherState& GetCurrentState() { return CurrentState; }
	FORCEINLINE const FAetherState& GetCurrentState() const { return CurrentState; }
	
	FORCEINLINE const FAetherState& GetLastState() const { return LastState; }
	
	const float& GetSinceLastTickTime() const { return SinceLastTickTime; }
	void IncSinceLastTickTime(const float& DeltaTime) { SinceLastTickTime += DeltaTime; }
	void ResetSinceLastTickTime() { SinceLastTickTime = 0.0f; }
};