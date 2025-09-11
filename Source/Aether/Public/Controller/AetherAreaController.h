/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"

#include "AetherControllerBase.h"
#include "AetherTypes.h"

#include "AetherAreaController.generated.h"

#if WITH_EDITORONLY_DATA
UCLASS(NotBlueprintable)
class UAetherAreaControllerVisualizeComponent : public UActorComponent
{
	GENERATED_BODY()
};
#endif

UCLASS()
class AETHER_API AAetherAreaController : public AAetherControllerBase
{
	GENERATED_BODY()
	
	friend class UAetherWorldSubsystem;
	
protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UAetherAreaControllerVisualizeComponent> VisualizeComponent;
#endif
	
	//~ Begin System Config Property
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|System")
	float AffectRadius;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|System")
	TObjectPtr<class UAetherSystemPreset> EarthLocationPreset;
#endif
	//~ End System Config Property
	
	//~ Begin Diel Rhythm Property
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Diel Rhythm")
	float DaytimeSpeedScale;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Diel Rhythm")
	float NightSpeedScale;
	//~ End Diel Rhythm Property
	
	//~ Begin Weather Property
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Weather")
	TArray<struct FWeatherEventDescription> PossibleWeatherEvents;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aether|Weather")
	TArray<FWeatherEventDescription> SubWeatherEvents;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Weather")
	TArray<FWeatherEventDescription> ExternalWeatherEvents;
	
	UPROPERTY(Transient, DuplicateTransient)
	TArray<TObjectPtr<class UAetherWeatherEventInstance>> ActiveWeatherInstance;
	
	// Cache
	UPROPERTY(Transient, DuplicateTransient)
	FGameplayTagContainer ActiveWeatherTags;
	
	// Cache
	UPROPERTY(Transient, DuplicateTransient)
	FGameplayTagContainer BlockingWeatherTags;
	//~ End Weather Property
	
	//~ Begin Evaporation Property
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Evaporation")
	float EvaporationCapacity;
	
	float SurfaceWater;
	//~ End Evaporation Property
	
	//~ Begin State Property
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|State")
	FGameplayTagContainer InitWeatherEventTags;
	
	UPROPERTY(Transient, DuplicateTransient, VisibleAnywhere, BlueprintReadWrite, Category = "Aether|State")
	FAetherState CurrentState;
	
	UPROPERTY(Transient, DuplicateTransient)
	FAetherState LastState;
	//~ End State Property
	
	float SinceLastTickTime;
	
	int32 TestCount = 0;
	
public:
	AAetherAreaController();
	
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	
	virtual void PostLoad() override;
	//~ End UObject Interface
	
#if UE_ENABLE_DEBUG_DRAWING
	void DrawDebugPointInfo(const FColor& Color) const;
#endif
	
protected:
	virtual void InitializeController() override;
	
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
	
protected:
	virtual void CalcSurfaceCoeffcient(float DeltaTime);
	
private:
#if WITH_EDITOR
	UFUNCTION(CallInEditor, Category = "Aether")
	void SyncOtherControllerDielRhythm();
	
	UFUNCTION(CallInEditor, Category = "Aether")
	void CorrectOtherControllerInitTimeStamp();
#endif
	
public:
	FORCEINLINE const float& GetAffectRadius() const { return AffectRadius; }
	
	FORCEINLINE FAetherState& GetCurrentState() { return CurrentState; }
	FORCEINLINE const FAetherState& GetCurrentState() const { return CurrentState; }
	
	FORCEINLINE const FAetherState& GetLastState() const { return LastState; }
	
	const float& GetSinceLastTickTime() const { return SinceLastTickTime; }
	void IncSinceLastTickTime(const float& DeltaTime) { SinceLastTickTime += DeltaTime; }
	void ResetSinceLastTickTime() { SinceLastTickTime = 0.0f; }
};