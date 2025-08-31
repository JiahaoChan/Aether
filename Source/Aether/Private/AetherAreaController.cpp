/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherAreaController.h"

#include "Components/BillboardComponent.h"

#include "AetherStats.h"
#include "AetherWeatherEvent.h"
#include "AetherWorldSubsystem.h"
#include "AetherSystemPreset.h"

static TAutoConsoleVariable<int32> CVarVisualizeAetherControllerState(
	TEXT("a.VisualizeAetherControllerState"),
	0,
	TEXT("Visualize the State of the Aether Area Controller."));

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

AAetherAreaController::AAetherAreaController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	
	bEnableAutoLODGeneration = false;
#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif
	
	USceneComponent* ControllerRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ControllerRoot"));
	RootComponent = ControllerRootComponent;
	
#if WITH_EDITORONLY_DATA
	UBillboardComponent * SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	
	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTextureObject;
			FName ID_Aether;
			FText NAME_Aether;
			FConstructorStatics()
				: SpriteTextureObject(TEXT("/Aether/Icons/S_ParticleSystem"))
				, ID_Aether(TEXT("Aether"))
				, NAME_Aether(NSLOCTEXT("SpriteCategory", "Aether", "Aether"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;
		
		if (SpriteComponent)
		{
			SpriteComponent->Sprite = ConstructorStatics.SpriteTextureObject.Get();
			SpriteComponent->SetRelativeScale3D_Direct(FVector(0.5f, 0.5f, 0.5f));
			SpriteComponent->bHiddenInGame = true;
			SpriteComponent->bIsScreenSizeScaled = true;
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Aether;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Aether;
			SpriteComponent->SetupAttachment(RootComponent);
			SpriteComponent->bReceivesDecals = false;
		}
	}
#endif // WITH_EDITORONLY_DATA
	
	bSimulateInGame = true;
#if WITH_EDITORONLY_DATA
	bSimulateInEditor = false;
#endif
	
	AffectRadius = 1000.0f;
	SimulationPlanet = ESimulationPlanetType::Earth;
	
#if WITH_EDITORONLY_DATA
	EarthLocationPreset = nullptr;
#endif
	
	Latitude = 0.0f;
	Longitude = 0.0f;
	PeriodOfDay = 2880.0f;
	DaysOfMonth = 8;
	DaytimeSpeedScale = 1.0f;
	NightSpeedScale = 1.0f;
	NorthDirectionYawOffset = 0.0f;
	
	InitTimeStampOfYear = 0.0f;
	InitWeatherEventTags = FGameplayTagContainer::EmptyContainer;
	
	SinceLastTickTime = 0.0;
}

void AAetherAreaController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		Subsystem->RegisterController(this);
	}
	
	Initialize();
}

void AAetherAreaController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (CVarVisualizeAetherControllerState.GetValueOnAnyThread() != 0)
	{
		FString DebugString = CurrentState.ToString();
		DebugString = GetActorLabel() + "\n" + DebugString;
		DrawDebugString(GetWorld(), GetActorLocation(), DebugString, nullptr, SinceLastTickTime > 0.033333f ? FColor::Orange : FColor::Green, 0.0f, true, 1.0f);
	}
#else
	SetActorTickEnabled(false);
#endif
}

void AAetherAreaController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
            {
            	Subsystem->RegisterController(this);
            }
		}
	}
#endif
	
	Initialize();
}

#if WITH_EDITOR
void AAetherAreaController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
	const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;
	
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, SimulationPlanet))
	{
		if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
		{
			Subsystem->ModifyAllControllersSimulationPlanetType_Editor(SimulationPlanet);
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, EarthLocationPreset))
	{
		if (EarthLocationPreset)
		{
			Latitude = EarthLocationPreset->Latitude;
			Longitude = EarthLocationPreset->Longitude;
			PossibleWeatherEvents.Empty();
			PossibleWeatherEvents = EarthLocationPreset->PossibleWeatherEvents;
			EarthLocationPreset = nullptr;
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, Latitude))
	{
		Latitude = FMath::Fmod(FMath::Fmod(Latitude + 90.0f, 180.0f) + 180.0 , 180.0f) - 90.0f;
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, Longitude))
	{
		Longitude = FMath::Fmod(FMath::Fmod(Longitude + 180.0f, 360.0f) + 360.0 , 360.0f) - 180.0f;
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, PeriodOfDay))
	{
		SyncOtherControllerDielRhythm();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, DaysOfMonth))
	{
		SyncOtherControllerDielRhythm();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, DaytimeSpeedScale) || MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, NightSpeedScale))
	{
		SyncOtherControllerDielRhythm();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, NorthDirectionYawOffset))
	{
		NorthDirectionYawOffset =  FMath::Fmod(NorthDirectionYawOffset + 360.0f, 360.0f);
		SyncOtherControllerDielRhythm();
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, InitTimeStampOfYear))
	{
		
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherAreaController, PossibleWeatherEvents))
	{
		for (FWeatherEventDescription& Description : PossibleWeatherEvents)
		{
			Description.UpdateHappeningMonthDisplayString();
			Description.FixProbability();
		}
	}
}

bool AAetherAreaController::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AAetherAreaController, SimulationPlanet))
	{
		
	}
	return Super::CanEditChange(InProperty);
}
#endif

void AAetherAreaController::PostLoad()
{
	Super::PostLoad();
	
#if WITH_EDITORONLY_DATA
	for (FWeatherEventDescription& Description : PossibleWeatherEvents)
	{
		Description.UpdateHappeningMonthDisplayString();
	}
#endif
}

void AAetherAreaController::Destroyed()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		Subsystem->UnRegisterController(this);
	}
	
	Super::Destroyed();
}

void AAetherAreaController::TickAetherController(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_AetherController_Tick);
	
	// Cache first.
	LastState = CurrentState;
	
	float ActualDeltaTime = SinceLastTickTime + DeltaTime;
	SinceLastTickTime = DeltaTime;
	
#if WITH_EDITORONLY_DATA
	const UWorld * World = GetWorld();
	if (World && (bSimulateInEditor && World->WorldType == EWorldType::Type::Editor || bSimulateInGame && World->IsGameWorld()))
#else
	if (bSimulationInGame)
#endif
	{
		CalcControllerState_DielRhythm(ActualDeltaTime);
		
        EvaluateWeatherEvent(ActualDeltaTime);
        
        UpdateWeatherEvent(ActualDeltaTime);
	}
}

void AAetherAreaController::CalcControllerState_DielRhythm(float DeltaTime)
{
	CalcSystemState_DielRhythm_Earth(DeltaTime);
}

void AAetherAreaController::CalcSystemState_DielRhythm_Earth(float DeltaTime)
{
	float DielDeltaTime = DeltaTime * (CurrentState.SunLightDirection.Z < 0.0f ? DaytimeSpeedScale : NightSpeedScale);
	CurrentState.ProgressOfYear += DielDeltaTime / (PeriodOfDay * DaysOfMonth * 12);
	CurrentState.ProgressOfYear = FMath::Frac(CurrentState.ProgressOfYear);
	
	CurrentState.Time += DeltaTime;
	
	UpdateSunByTime();
}

void AAetherAreaController::UpdateSunByTime()
{
	int32 PolarCondition = 0;
	float SunElevation = 0.0f;
	float SunAzimuth = 0.0f;
	const float ProgressOfDay = FMath::Frac(CurrentState.ProgressOfYear * DaysOfMonth * 12);
	CalculateSunPosition(
		Latitude,
		Longitude,
		ProgressOfDay * SECONDS_PER_DAY_EARTH,
		CurrentState.ProgressOfYear * SECONDS_PER_YEAR_EARTH,
		SunElevation,
		SunAzimuth,
		PolarCondition);
	
	CurrentState.SunElevation = SunElevation;
	CurrentState.SunAzimuth = SunAzimuth;
	CurrentState.SunLightDirection = ConvertPlanetLightDirection(SunElevation, SunAzimuth);
}

void AAetherAreaController::CalcSystemState_DielRhythm_Custom(float DeltaTime)
{
	
}

void AAetherAreaController::EvaluateWeatherEvent(float DeltaTime)
{
	TArray<int32> DeferredTriggerEventIndexList;
	for (int32 i = 0; i < PossibleWeatherEvents.Num(); i++)
	{
		const FWeatherEventDescription& EventDescription = PossibleWeatherEvents[i];
		if (EventDescription.TriggerSource != EWeatherTriggerSource::AetherController || !EventDescription.Event)
		{
			continue;
		}
		if (EventDescription.Event->EventTag.HasAnyExact(ActiveWeatherTags))
		{
			// Never trigger the same weather event twice in a period of time.
			continue;
		}
		if (EventDescription.Event->EventTag.HasAny(BlockingWeatherTags))
		{
			// Check current running event if blocks the incoming event.
			continue;
		}
		if (EventDescription.HappeningMonthsProbability.FindRef(CurrentState.Month, 0.0f) <= UE_KINDA_SMALL_NUMBER)
		{
			continue;
		}
		DeferredTriggerEventIndexList.Add(i);
	}
	
	// Check if blocking or canceling each other.
	TArray<int32> TriggerEventIndexList;
	
	
	// Todo
	// Check if blocked or canceled by active instance.

	// Todo: incoming event cancel active event
	FGameplayTagContainer DeferredCancelEventTags;

	
	// Cancel
	TArray<UAetherWeatherEventInstance*> DeferredCancelEventInstances;
	for (UAetherWeatherEventInstance* Instance : ActiveWeatherInstance)
	{
		if (Instance && Instance->State == EWeatherEventExecuteState::Running)
		{
			check(Instance->EventClass);
			if (Instance->EventClass->EventTag.HasAny(DeferredCancelEventTags))
			{
				DeferredCancelEventInstances.Add(Instance);
			}
		}
	}
	for (UAetherWeatherEventInstance* Instance : DeferredCancelEventInstances)
	{
		if (Instance)
		{
			CancelWeatherEventImmediately(Instance);
		}
	}
	
	// Try to trigger
	for (const int32& Index : DeferredTriggerEventIndexList)
	{
		check(PossibleWeatherEvents.IsValidIndex(Index));
		UAetherWeatherEventInstance* NewInstance = PossibleWeatherEvents[Index].Event->MakeInstance_Route(this);
		//if (CurrentState.RunningWeatherTags.HasAllExact(PossibleWeatherEvents[Index].Event->PreConditionWeatherEventsWithTag))
		{
			//NewInstance->SetState(EWeatherEventExecuteState::BlendingIn, this);
			//RunningWeatherInstance.Add(NewInstance);
		}
		//else
		{
			// Todo: 进一步触发?
			//NewInstance->SetState(EWeatherEventExecuteState::Warming, this);
			//WarmingWeatherInstance.Add(NewInstance);
		}
	}
	
	// Kick out finished instance.
	for (int32 i = ActiveWeatherInstance.Num() - 1; i >= 0; i--)
	{
		if (ActiveWeatherInstance[i] && ActiveWeatherInstance[i]->State == EWeatherEventExecuteState::Finished)
		{
			ActiveWeatherInstance.RemoveAt(i);
		}
	}
}

void AAetherAreaController::UpdateWeatherEvent(float DeltaTime)
{
	auto SetNewState = [this](UAetherWeatherEventInstance* InInstance, const EWeatherEventExecuteState& NewState)
	{
		if (!InInstance || InInstance->State == NewState)
		{
			return;
		}
		check(InInstance->EventClass);
		if (InInstance->State == EWeatherEventExecuteState::JustSpawned && NewState != EWeatherEventExecuteState::Finished)
		{
			ActiveWeatherTags.AppendTags(InInstance->EventClass->EventTag);
			BlockingWeatherTags.AppendTags(InInstance->EventClass->BlockWeatherEventsWithTag);
		}
		else if (NewState == EWeatherEventExecuteState::Finished)
		{
			ActiveWeatherTags.RemoveTags(InInstance->EventClass->EventTag);
			BlockingWeatherTags.RemoveTags(InInstance->EventClass->BlockWeatherEventsWithTag);
		}
		InInstance->State = NewState;
		InInstance->CurrentStateLastTime = 0.0f;
	};
	
	for (UAetherWeatherEventInstance* Instance : ActiveWeatherInstance)
	{
		if (Instance)
		{
			check(Instance->EventClass);
			const EWeatherEventExecuteState LastExecuteState = Instance->State;
			switch (LastExecuteState)
			{
				case EWeatherEventExecuteState::JustSpawned:
					{
						SetNewState(Instance, EWeatherEventExecuteState::BlendingIn);
						break;
					}
				case EWeatherEventExecuteState::BlendingIn:
					{
						const EWeatherEventExecuteState NewState = Instance->BlendIn(DeltaTime, this);
						if (LastExecuteState != NewState)
						{
							SetNewState(Instance, NewState);
						}
						else
						{
							Instance->CurrentStateLastTime += DeltaTime;
						}
						break;
					}
				case EWeatherEventExecuteState::Running:
					{
						const EWeatherEventExecuteState NewState = Instance->Run(DeltaTime, this);
						if (LastExecuteState != NewState)
						{
							SetNewState(Instance, NewState);
						}
						else
						{
							Instance->CurrentStateLastTime += DeltaTime;
						}
						break;
					}
				case EWeatherEventExecuteState::BlendingOut:
					{
						const EWeatherEventExecuteState NewState = Instance->BlendOut(DeltaTime, this);
						if (LastExecuteState != NewState)
						{
							SetNewState(Instance, NewState);
						}
						else
						{
							Instance->CurrentStateLastTime += DeltaTime;
						}
						break;
					}
				default:
					check(false);
					break;
			}
		}
	}
}

void AAetherAreaController::Initialize()
{
	CurrentState.Reset();
	CurrentState.Latitude = Latitude;
	CurrentState.Longitude = Longitude;
	LastState = CurrentState;
	ActiveWeatherInstance.Reset();
	ActiveWeatherTags.Reset();
	BlockingWeatherTags.Reset();
	SinceLastTickTime = 0.0f;
	
	CurrentState.ProgressOfYear = InitTimeStampOfYear / (PeriodOfDay * DaysOfMonth * 12);
	CurrentState.ProgressOfYear = FMath::Frac(CurrentState.ProgressOfYear);
	
	CalcControllerState_DielRhythm(0.0f);
}

#if WITH_EDITOR
void AAetherAreaController::CaptureTimeStamp()
{
	InitTimeStampOfYear = CurrentState.ProgressOfYear * (PeriodOfDay * DaysOfMonth * 12);
}

void AAetherAreaController::SyncOtherControllerDielRhythm()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		Subsystem->SyncOtherControllerDielRhythm_Editor(this);
	}
}

void AAetherAreaController::CorrectOtherControllerInitTimeStamp()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		Subsystem->CorrectOtherControllerInitTimeStamp_Editor(this);
	}
}
#endif

void AAetherAreaController::TriggerWeatherEventImmediately(const FGameplayTag& EventTag)
{
	
}

void AAetherAreaController::TriggerWeatherEventImmediately(const FGameplayTagContainer& EventTags)
{
	
}

void AAetherAreaController::TriggerWeatherEventImmediately(const UAetherWeatherEvent* EventClass)
{
	
}

void AAetherAreaController::CancelWeatherEventImmediately(const FGameplayTag& EventTag)
{
	
}

void AAetherAreaController::CancelWeatherEventImmediately(const FGameplayTagContainer& EventTags)
{
	
}

void AAetherAreaController::CancelWeatherEventImmediately(const UAetherWeatherEvent* EventClass)
{
	
}

void AAetherAreaController::CancelWeatherEventImmediately(UAetherWeatherEventInstance* EventInstance)
{
	if (!EventInstance)
	{
		return;
	}
	if (EventInstance->State == EWeatherEventExecuteState::Running)
	{
		check(EventInstance->EventClass);
		RunningWeatherTags.RemoveTags(EventInstance->EventClass->EventTag);
		BlockingWeatherTags.RemoveTags(EventInstance->EventClass->BlockWeatherEventsWithTag);
	}
	EventInstance->State = EWeatherEventExecuteState::BlendingOut;
	EventInstance->CurrentStateLastTime = 0.0f;
}

void AAetherAreaController::SetSimulationPlanet(const ESimulationPlanetType& NewValue)
{
	if (SimulationPlanet != NewValue)
	{
		SimulationPlanet = NewValue;
	}
}