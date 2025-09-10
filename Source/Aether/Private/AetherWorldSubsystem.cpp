/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherWorldSubsystem.h"

#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

#include "AetherAreaController.h"
#include "AetherCloudAvatar.h"
#include "AetherLightingAvatar.h"
#include "AetherPluginSettings.h"
#include "AetherSettingsInfo.h"
#include "AetherStats.h"

#if UE_ENABLE_DEBUG_DRAWING
static TAutoConsoleVariable<int32> CVarVisualizeAetherState(
	TEXT("a.VisualizeAetherState"),
	0,
	TEXT("Visualize the State of the Aether System."),
	ECVF_Cheat);

static TAutoConsoleVariable<int32> CVarVisualizeAetherControllerWeight(
	TEXT("a.VisualizeAetherControllerWeight"),
	0,
	TEXT("Visualize the distance-based weight of Aether Controllers."),
	ECVF_Cheat);

static TAutoConsoleVariable<int32> CVarVisualizeAetherControllerPoint(
	TEXT("a.VisualizeAetherControllerPoint"),
	0,
	TEXT("Visualize the location point of Aether Controllers."),
	ECVF_Cheat);
#endif

#define SECONDS_PER_HOUR 3600.0f
#define SECONDS_PER_DAY_EARTH 86400.0f
#define SECONDS_PER_YEAR_EARTH 31536000.0f
#define DAYS_PER_YEAR_EARTH 365

#define OBLIQUITY 23.45f				// 黄赤交角（度）
#define STANDARD_MERIDIAN 120.0f		// 标准时区经度（东八区）
#define DEG_TO_HOUR 15.0f				// 度到小时转换系数（1小时=15度）

/**
 * 计算指定地理位置和时间的太阳位置（高度角、方位角）及极地条件
 * @param Latitude 纬度（度），北纬为正
 * @param Longitude 经度（度），东经为正
 * @param TimeStampOfEarthDay 当日累计秒数（0~86400）
 * @param TimeStampOfEarthYear 当年累计秒数（0~31536000）
 * @param SunElevation [输出] 太阳高度角（度），地平线以上为正值
 * @param SunAzimuth [输出] 太阳方位角（度），从正北顺时针测量
 * @param PolarCondition [输出] 极地条件标志：0=正常，1=极昼，-1=极夜
 */
void CalculateSunPosition(
    const float& Latitude,
    const float& Longitude,
    const float& TimeStampOfEarthDay,
    const float& TimeStampOfEarthYear,
    float& SunElevation,
    float& SunAzimuth,
    int32& PolarCondition)
{
    // ===== 1. 输入预处理 =====
    // 计算年序日（Day of Year, 1-365）
    float DateOfYear = FMath::Fmod(TimeStampOfEarthYear / SECONDS_PER_DAY_EARTH, DAYS_PER_YEAR_EARTH);
    
    // 将秒转换为小时（地方时）
    float LocalHour = TimeStampOfEarthDay / SECONDS_PER_HOUR;
    
    // ===== 2. 天文参数计算 =====
    // 计算太阳赤纬角（δ），单位：弧度（使用标准近似公式）
    float SolarDeclination = FMath::DegreesToRadians(OBLIQUITY * FMath::Sin(2 * PI * (284.0f + DateOfYear) / DAYS_PER_YEAR_EARTH));
    
    // 计算时角（H），单位：弧度
    // a. 经度转换为时区修正（15度=1小时）
    float TimeZoneCorrection = (Longitude - STANDARD_MERIDIAN) / DEG_TO_HOUR;
    // b. 计算真太阳时
    float LocalSolarTime = LocalHour + TimeZoneCorrection;
    // c. 计算时角（度）并转换为弧度
    float HourAngleDeg = DEG_TO_HOUR * (LocalSolarTime - 12.0f);
    float HourAngleRad = FMath::DegreesToRadians(HourAngleDeg);
    
    // 纬度转换为弧度
    float LatRad = FMath::DegreesToRadians(Latitude);
    
    // ===== 3. 太阳高度角计算 =====
    // 使用标准球面三角公式：sin(α) = sin(φ)sin(δ) + cos(φ)cos(δ)cos(H)
    float SinElevation = FMath::Sin(LatRad) * FMath::Sin(SolarDeclination) 
                       + FMath::Cos(LatRad) * FMath::Cos(SolarDeclination) * FMath::Cos(HourAngleRad);
    
    // 处理浮点精度溢出，确保值在[-1, 1]范围内
    SinElevation = FMath::Clamp(SinElevation, -1.0f, 1.0f);
    
    // 计算高度角弧度值（后续计算会复用）
    float ElevationRad = FMath::Asin(SinElevation);
    SunElevation = FMath::RadiansToDegrees(ElevationRad);
    
    // ===== 4. 太阳方位角计算 =====
    // 公式：cos(Az) = [sin(δ) - sin(α)sin(φ)] / [cos(α)cos(φ)]
	// 计算方位角的正弦和余弦分量
	float sinAzimuth = -FMath::Cos(SolarDeclination) * FMath::Sin(HourAngleRad);
	float cosAzimuth = FMath::Sin(SolarDeclination) * FMath::Cos(LatRad) 
					 - FMath::Cos(SolarDeclination) * FMath::Sin(LatRad) * FMath::Cos(HourAngleRad);
	
	// 使用atan2计算方位角（弧度）
	float azimuthRad = FMath::Atan2(sinAzimuth, cosAzimuth);
	
	// 转换为度数并确保在0-360度范围内
	SunAzimuth = FMath::RadiansToDegrees(azimuthRad);
	SunAzimuth = FMath::Fmod(SunAzimuth + 360.0f, 360.0f);
	/*
	// 特殊处理：如果高度角为0，进行微调以避免数值不稳定
	if (FMath::Abs(SunElevation) < 0.1f) {
		// 根据时角确定日出日落方向
		if (HourAngleRad < 0) {
			SunAzimuth = 90.0f; // 日出方向（东）
		} else {
			SunAzimuth = 270.0f; // 日落方向（西）
		}
	}
    */
	
    // ===== 5. 极昼/极夜检测 =====
    // 计算临界角余弦值（用于判断太阳是否可能升起）
    float CriticalAngleCos = -FMath::Sin(LatRad) * FMath::Sin(SolarDeclination) / (FMath::Cos(LatRad) * FMath::Cos(SolarDeclination));
    
    // 判断逻辑
    PolarCondition = 0; // 默认正常
    if (CriticalAngleCos > 1.0f)
    {
        PolarCondition = -1; // 极夜（全天无日出）
        SunElevation = -90.0f; // 太阳位于地平线以下
    }
	else if (CriticalAngleCos < -1.0f)
	{
        PolarCondition = 1;  // 极昼（全天有日照）
        SunElevation = 90.0f; // 太阳位于头顶
    }
}

/* Vector Pivot: Local Position, Vector Point at: Planet. */
FRotator ConvertPlanetRotation(const float& ElevationDegree, const float& AzimuthDegree)
{
	FRotator Rotator = FRotator::ZeroRotator;
	Rotator.Pitch = ElevationDegree;
	Rotator.Yaw = AzimuthDegree;
	return Rotator;
}

/* Vector Pivot: Local Position, Vector Point at: Planet. */
FVector ConvertPlanetDirection(const float& ElevationDegree, const float& AzimuthDegree)
{
	return ConvertPlanetRotation(ElevationDegree, AzimuthDegree).Vector();
}

/* Vector Pivot: Planet, Vector Point at: Local Position. */
FVector ConvertPlanetLightDirection(const float& ElevationDegree, const float& AzimuthDegree)
{
	return -ConvertPlanetDirection(ElevationDegree, AzimuthDegree);
}

UAetherWorldSubsystem::UAetherWorldSubsystem()
{
	LightingAvatar = nullptr;
	CloudAvatar = nullptr;
	SystemMaterialParameterCollection = nullptr;
}

bool UAetherWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}
	TArray<UClass*> DerivedClassList;
	GetDerivedClasses(GetClass(), DerivedClassList, false);
	return DerivedClassList.Num() == 0;
}

void UAetherWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UAetherPluginSettings* Settings = GetDefault<UAetherPluginSettings>();
	
	if (UMaterialParameterCollection* ParameterCollection = Settings->SystemMaterialParameterCollection.LoadSynchronous())
	{
		SystemMaterialParameterCollection = ParameterCollection;
	}
	else
	{
		// Todo: Log
	}
	
	SettingsInfo = nullptr;
	Controllers.Empty();
	ActiveControllers.Empty();
	Avatars.Empty();
	LightingAvatar = nullptr;
	CloudAvatar = nullptr;
	SystemState.Reset();
	StreamingSourceLocation = FVector4f::Zero();
	StreamingSourceLocation.W = -1.0f;
	
#if WITH_EDITOR
	if (UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			FEditorDelegates::OnMapOpened.AddUObject(this, &UAetherWorldSubsystem::OnMapOpened);
		}
	}
#endif
}

bool UAetherWorldSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::Editor || WorldType == EWorldType::PIE;
}

bool UAetherWorldSubsystem::IsTickable() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
#if WITH_EDITOR
	if (World->WorldType == EWorldType::Type::Editor && GEditor)
	{
		if (const FViewport* Viewport = GEditor->GetActiveViewport())
		{
			// Editor world still maintains when pulling up a PIE world, but it can not be ever tick controller or system.
			if (Viewport->IsPlayInEditorViewport())
			{
				return false;
			}
		}
	}
#endif
	if (World->IsGameWorld() && !World->HasBegunPlay())
	{
		return false;
	}
	if (World->IsGameWorld())
	{
		if (SettingsInfo && !SettingsInfo->bSimulateInGame)
		{
			return false;
		}
	}
#if WITH_EDITOR
	else
	{
		if (SettingsInfo && !SettingsInfo->bSimulateInEditor)
		{
			return false;
		}
	}
#endif
	return true;
}

void UAetherWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SCOPE_CYCLE_COUNTER(STAT_AetherWorldSubsystem_Tick);
	
	EvaluateActiveControllers();
	UpdateSourceCoordinate();
	UpdateSystemState_DielRhythm(DeltaTime);
	UpdateSystemStateFromActiveControllers(DeltaTime);
	UpdateWorld();
	
#if UE_ENABLE_DEBUG_DRAWING
	if (CVarVisualizeAetherState.GetValueOnGameThread() > 0 && GEngine)
	{
		FString DebugString = FString("Aether System State:\n") + SystemState.ToString();
		GEngine->AddOnScreenDebugMessage((uint64)this + 0, 1.0f, FColor::Green, DebugString);
	}
	
	if (CVarVisualizeAetherControllerWeight.GetValueOnGameThread() > 0 && GEngine)
	{
		FString DebugString = FString("Aether Controllers weight map:\n");
		for (auto It = ActiveControllers.CreateConstIterator(); It; ++It)
		{
			DebugString += It.Key()->GetActorLabel() + FString(": ") + FString::SanitizeFloat(It.Value()) + FString("\n");
		}
		GEngine->AddOnScreenDebugMessage((uint64)this + 1, 1.0f, FColor::Green, DebugString);
	}
	
	if (CVarVisualizeAetherControllerPoint.GetValueOnGameThread() > 0 && GEngine)
	{
		for (auto It = ActiveControllers.CreateConstIterator(); It; ++It)
		{
			It.Key()->DrawDebugPointInfo(FMath::Lerp(FLinearColor::Red, FLinearColor::Green, It.Value()).ToFColor(true));
		}
	}
#endif
}

void UAetherWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	// This is called earlier than any Actor's BeginPlay that does register work.
	//InitializeAetherSystem();
	InWorld.OnWorldBeginPlay.AddUObject(this, &UAetherWorldSubsystem::PostWorldBeginPlay);
}

UAetherWorldSubsystem* UAetherWorldSubsystem::Get(UObject* ContextObject)
{
	UAetherWorldSubsystem* Subsystem = Cast<UAetherWorldSubsystem>(USubsystemBlueprintLibrary::GetWorldSubsystem(ContextObject, StaticClass()));
	return Subsystem;
}

void UAetherWorldSubsystem::RegisterGlobalSettings(AAetherSettingsInfo* InSettingsInfo)
{
	if (!InSettingsInfo)
	{
		return;
	}
	SettingsInfo = InSettingsInfo;
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			// Refresh for Editor World.
			InitializeAetherSystem();
		}
	}
#endif
}

void UAetherWorldSubsystem::UnregisterGlobalSettings(AAetherSettingsInfo* InSettingsInfo)
{
	if (!InSettingsInfo)
	{
		return;
	}
	if (SettingsInfo == InSettingsInfo)
	{
		SettingsInfo = nullptr;
	}
}

void UAetherWorldSubsystem::RegisterController(AAetherAreaController* InController)
{
	if (!InController)
	{
		return;
	}
	Controllers.AddUnique(InController);
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			// Refresh for Editor World.
			InitializeAetherSystem();
		}
	}
#endif
}

void UAetherWorldSubsystem::UnregisterController(AAetherAreaController* InController)
{
	if (!InController)
	{
		return;
	}
	Controllers.Remove(InController);
	ActiveControllers.Remove(InController);
}

void UAetherWorldSubsystem::RegisterAvatar(AAetherAvatarBase* InAvatar)
{
	if (!InAvatar)
	{
		return;
	}
	if (AAetherLightingAvatar* InLightingAvatar = Cast<AAetherLightingAvatar>(InAvatar))
	{
		LightingAvatar = InLightingAvatar;
	}
	else if (AAetherCloudAvatar* InCloudAvatar = Cast<AAetherCloudAvatar>(InAvatar))
	{
		CloudAvatar = InCloudAvatar;
	}
	Avatars.AddUnique(InAvatar);
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			// Refresh for Editor World.
			InitializeAetherSystem();
		}
	}
#endif
}

void UAetherWorldSubsystem::UnregisterAvatar(AAetherAvatarBase* InAvatar)
{
	if (!InAvatar)
	{
		return;
	}
	if (LightingAvatar == InAvatar)
	{
		LightingAvatar = nullptr;
	}
	else if (CloudAvatar == InAvatar)
	{
		CloudAvatar = nullptr;
	}
	Avatars.Remove(InAvatar);
}

void UAetherWorldSubsystem::TriggerWeatherEventImmediately(const FGameplayTag& EventTag)
{
	
}

void UAetherWorldSubsystem::InitializeAetherSystem()
{
	SystemState.Reset();
	StreamingSourceLocation = FVector4f::Zero();
	StreamingSourceLocation.W = -1.0f;
	EvaluateActiveControllers();
	UpdateSourceCoordinate();
	if (SettingsInfo)
	{
		SystemState.ProgressOfYear = FMath::Frac(SettingsInfo->InitTimeStampOfYear / (SettingsInfo->PeriodOfDay * SettingsInfo->DaysOfMonth * 12));
	}
	UpdateSystemState_DielRhythm(0.0f);
	UpdateSystemStateFromActiveControllers(0.0f);
	UpdateWorld();
}

void UAetherWorldSubsystem::PostWorldBeginPlay()
{
	// Game Word initialize once. All the actors have registered.
	InitializeAetherSystem();
}

#if WITH_EDITOR
void UAetherWorldSubsystem::OnMapOpened(const FString& Filename, bool bAsTemplate)
{
	if (!bAsTemplate)
	{
		// Game Word initialize once. All the actors have registered.
		InitializeAetherSystem();
	}
}
#endif

void UAetherWorldSubsystem::EvaluateActiveControllers()
{
	ActiveControllers.Reset();
	
	StreamingSourceLocation.W = -1.0f;
	if (UWorld* World = GetWorld())
	{
#if WITH_EDITOR
		if (GEditor && World->WorldType == EWorldType::Type::Editor)
		{
			if (const FViewport* Viewport = GEditor->GetActiveViewport())
			{
				if (FViewportClient* ViewportClient = Viewport->GetClient())
				{
					if (const FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(ViewportClient))
					{
						FVector ViewLocation = EditorViewportClient->GetViewLocation();
						StreamingSourceLocation.Set(ViewLocation.X, ViewLocation.Y, ViewLocation.Z, 1.0f);
					}
				}
			}
		}
		else
		{
#endif
			check(World->IsGameWorld());
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				if (const AActor* ViewTarget = PlayerController->GetViewTarget())
				{
					FVector ViewTargetLocation = ViewTarget->GetActorLocation();
					StreamingSourceLocation.Set(ViewTargetLocation.X, ViewTargetLocation.Y, ViewTargetLocation.Z, 1.0f);
				}
				else
				{
					FVector ViewPointLocation;
					FRotator Rotation;
					PlayerController->GetPlayerViewPoint(ViewPointLocation, Rotation);
					StreamingSourceLocation.Set(ViewPointLocation.X, ViewPointLocation.Y, ViewPointLocation.Z, 1.0f);
				}
			}
#if WITH_EDITOR
		}
#endif
	}
	
	if (StreamingSourceLocation.W > 0.0f)
	{
		float WeightSum = 0.0f;
		TMap<AAetherAreaController*, float> ControllerDistanceMap;
		for (AAetherAreaController* Controller : Controllers)
		{
			if (Controller)
			{
				float Dis = FVector::Distance(Controller->GetActorLocation(), FVector(StreamingSourceLocation));
				Dis = FMath::Max(Dis - Controller->GetAffectRadius(), UE_SMALL_NUMBER);
				float Weight = 1.0f / FMath::Pow(Dis, 2.0f);
				ControllerDistanceMap.Add(Controller, Weight);
				WeightSum += Weight;
			}
		}
		for (auto It = ControllerDistanceMap.CreateConstIterator(); It; ++It)
		{
			float NormalizedWeight = It.Value() / WeightSum;
			if (NormalizedWeight > UE_KINDA_SMALL_NUMBER)
			{
				ActiveControllers.Add(It.Key(), NormalizedWeight);
			}
		}
	}
}

void UAetherWorldSubsystem::UpdateSourceCoordinate()
{
	if (SettingsInfo && StreamingSourceLocation.W > 0.0f)
	{
		FVector Offset = FVector(StreamingSourceLocation) - SettingsInfo->GetActorLocation();
		Offset.Z = 0.0f;
		FVector NorthDirection = FRotator(0.0f, SettingsInfo->NorthDirectionYawOffset, 0.0f).Vector();
		FVector EastDirection = FRotator(0.0f, SettingsInfo->NorthDirectionYawOffset + 90.0f, 0.0f).Vector();
		float NorthDis = FVector::DotProduct(Offset, NorthDirection);
		float EastDis = FVector::DotProduct(Offset, EastDirection);
		SystemState.Latitude = SettingsInfo->Latitude + NorthDis / SettingsInfo->NorthDisPerDegreeLatitude / 100.0f;
		SystemState.Longitude = SettingsInfo->Longitude + EastDis / SettingsInfo->EastDisPerDegreeLongitude / 100.0;
	}
}

void UAetherWorldSubsystem::UpdateSystemState_DielRhythm(float DeltaTime)
{
	if (1)
	{
		UpdateSystemState_DielRhythm_Earth(DeltaTime);
	}
	else
	{
		UpdateSystemState_DielRhythm_Custom(DeltaTime);
	}
}

void UAetherWorldSubsystem::UpdateSystemState_DielRhythm_Earth(float DeltaTime)
{
	if (SettingsInfo)
	{
		float DaytimeSpeedScale = ActiveControllers.Num() > 0 ? 0.0f : 1.0f;
        float NightSpeedScale = ActiveControllers.Num() > 0 ? 0.0f : 1.0f;
        for (auto It = ActiveControllers.CreateConstIterator(); It; ++It)
        {
        	DaytimeSpeedScale += It.Key()->DaytimeSpeedScale * It.Value();
        	NightSpeedScale += It.Key()->NightSpeedScale * It.Value();
        }
        float DielDeltaTime = DeltaTime * (SystemState.SunLightDirection.Z < 0.0f ? DaytimeSpeedScale : NightSpeedScale);
        SystemState.ProgressOfYear += DielDeltaTime / (SettingsInfo->PeriodOfDay * SettingsInfo->DaysOfMonth * 12);
        SystemState.ProgressOfYear = FMath::Frac(SystemState.ProgressOfYear);
        // Todo
        SystemState.Time += DeltaTime;
	}
	UpdatePlanetByTime();
}

void UAetherWorldSubsystem::UpdatePlanetByTime()
{
	if (SettingsInfo)
	{
		int32 PolarCondition = 0;
        float SunElevation = 0.0f;
        float SunAzimuth = 0.0f;
        const float ProgressOfDay = FMath::Frac(SystemState.ProgressOfYear * SettingsInfo->DaysOfMonth * 12);
        CalculateSunPosition(
        	SystemState.Latitude,
        	SystemState.Longitude,
        	ProgressOfDay * SECONDS_PER_DAY_EARTH,
        	SystemState.ProgressOfYear * SECONDS_PER_YEAR_EARTH,
        	SunElevation,
        	SunAzimuth,
        	PolarCondition);
        SystemState.SunElevation = SunElevation;
        SystemState.SunAzimuth = SunAzimuth;
        SystemState.SunLightDirection = ConvertPlanetLightDirection(SunElevation, SunAzimuth);
	}
}

void UAetherWorldSubsystem::UpdateSystemState_DielRhythm_Custom(float DeltaTime)
{
	
}

void UAetherWorldSubsystem::UpdateSystemStateFromActiveControllers(float DeltaTime)
{
	EvaluateWeatherEvent(DeltaTime);
	UpdateWeatherEvent(DeltaTime);
	// Todo
	//CalcSurfaceCoeffcient(ActualDeltaTime);
}

void UAetherWorldSubsystem::EvaluateWeatherEvent(float DeltaTime)
{
	if (ActiveControllers.Num() > 0)
	{
		
	}
}

void UAetherWorldSubsystem::UpdateWeatherEvent(float DeltaTime)
{
	
}

void UAetherWorldSubsystem::UpdateWorld()
{
	UpdateAvatar();
	UpdateSystemMaterialParameter();
}

void UAetherWorldSubsystem::UpdateAvatar()
{
	for (AAetherAvatarBase* Avatar : Avatars)
	{
		if (Avatar)
		{
			Avatar->UpdateFromSystemState(SystemState);
		}
	}
}

void UAetherWorldSubsystem::UpdateSystemMaterialParameter()
{
	if (!SystemMaterialParameterCollection)
	{
		return;
	}
	{
		FLinearColor ExistValue = UKismetMaterialLibrary::GetVectorParameterValue(this, SystemMaterialParameterCollection, FName("SunLightDirection"));
		if (!SystemState.SunLightDirection.Equals(FVector(ExistValue.R, ExistValue.G, ExistValue.B)))
		{
			UKismetMaterialLibrary::SetVectorParameterValue(this, SystemMaterialParameterCollection, FName("SunLightDirection"), FLinearColor(SystemState.SunLightDirection.X, SystemState.SunLightDirection.Y, SystemState.SunLightDirection.Z, 0.0f));
		}
	}
	{
		FLinearColor ExistValue = UKismetMaterialLibrary::GetVectorParameterValue(this, SystemMaterialParameterCollection, FName("MoonLightDirection"));
		if (!SystemState.MoonLightDirection.Equals(FVector(ExistValue.R, ExistValue.G, ExistValue.B)))
		{
			UKismetMaterialLibrary::SetVectorParameterValue(this, SystemMaterialParameterCollection, FName("MoonLightDirection"), FLinearColor(SystemState.MoonLightDirection.X, SystemState.MoonLightDirection.Y, SystemState.MoonLightDirection.Z, 0.0f));
		}
	}
	{
		FLinearColor ExistValue = UKismetMaterialLibrary::GetVectorParameterValue(this, SystemMaterialParameterCollection, FName("WindData"));
		if (!SystemState.WindData.Equals(FVector4f(ExistValue.R, ExistValue.G, ExistValue.B, ExistValue.A)))
		{
			UKismetMaterialLibrary::SetVectorParameterValue(this, SystemMaterialParameterCollection, FName("WindData"), FLinearColor(SystemState.WindData.X, SystemState.WindData.Y, SystemState.WindData.Z, SystemState.WindData.W));
		}
	}
	
	{
		float ExistValue = UKismetMaterialLibrary::GetScalarParameterValue(this, SystemMaterialParameterCollection, FName("SurfaceRainRemain"));
		if (!FMath::IsNearlyEqual(SystemState.SurfaceRainRemain, ExistValue, UE_KINDA_SMALL_NUMBER))
		{
			UKismetMaterialLibrary::SetScalarParameterValue(this, SystemMaterialParameterCollection, FName("SurfaceRainRemain"), SystemState.SurfaceRainRemain);
		}
	}
	{
		float ExistValue = UKismetMaterialLibrary::GetScalarParameterValue(this, SystemMaterialParameterCollection, FName("SurfaceSnowDepth"));
		if (!FMath::IsNearlyEqual(SystemState.SurfaceSnowDepth, ExistValue, UE_KINDA_SMALL_NUMBER))
		{
			UKismetMaterialLibrary::SetScalarParameterValue(this, SystemMaterialParameterCollection, FName("SurfaceSnowDepth"), SystemState.SurfaceSnowDepth);
		}
	}
	{
		float ExistValue = UKismetMaterialLibrary::GetScalarParameterValue(this, SystemMaterialParameterCollection, FName("ProgressOfYear"));
		if (!FMath::IsNearlyEqual(SystemState.ProgressOfYear, ExistValue, UE_KINDA_SMALL_NUMBER))
		{
			UKismetMaterialLibrary::SetScalarParameterValue(this, SystemMaterialParameterCollection, FName("ProgressOfYear"), SystemState.ProgressOfYear);
		}
	}
}