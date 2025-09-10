/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherSettingsInfo.h"

#include "Components/BillboardComponent.h"

#include "AetherWorldSubsystem.h"

AAetherSettingsInfo::AAetherSettingsInfo()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bEnableAutoLODGeneration = false;
#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif
	
#if WITH_EDITORONLY_DATA
	USceneComponent* SettingsRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SettingsRoot"));
	RootComponent = SettingsRootComponent;
	
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
		
		if (UBillboardComponent * ParentSpriteComponent = GetSpriteComponent())
		{
			ParentSpriteComponent->Sprite = ConstructorStatics.SpriteTextureObject.Get();
			ParentSpriteComponent->SetRelativeScale3D_Direct(FVector(0.5f, 0.5f, 0.5f));
			ParentSpriteComponent->bHiddenInGame = true;
			ParentSpriteComponent->bIsScreenSizeScaled = true;
			ParentSpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Aether;
			ParentSpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Aether;
			ParentSpriteComponent->SetupAttachment(RootComponent);
			ParentSpriteComponent->bReceivesDecals = false;
		}
	}
#endif // WITH_EDITORONLY_DATA
	
#if WITH_EDITORONLY_DATA
	VisualizeComponent = CreateDefaultSubobject<UAetherSettingsVisualizeComponent>(TEXT("VisualizeComponent"));
#endif
	
	bSimulateInGame = true;
#if WITH_EDITORONLY_DATA
	bSimulateInEditor = false;
#endif
	SimulatePlanet = ESimulatePlanetType::Earth;
	Latitude = 0.0f;
	Longitude = 0.0f;
	NorthDisPerDegreeLatitude = 66.0f;
	EastDisPerDegreeLongitude = 66.0f;
	PeriodOfDay = 2880.0f;
	DaysOfMonth = 8;
	NorthDirectionYawOffset = 0.0f;
	InitTimeStampOfYear = 0.0f;
}

void AAetherSettingsInfo::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
            {
				Subsystem->RegisterGlobalSettings(this);
            }
		}
	}
#endif
}

void AAetherSettingsInfo::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		Subsystem->RegisterGlobalSettings(this);
	}
}

void AAetherSettingsInfo::Destroyed()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		Subsystem->UnregisterGlobalSettings(this);
	}
	Super::Destroyed();
}

#if WITH_EDITOR
void AAetherSettingsInfo::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
	const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;
	
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherSettingsInfo, Latitude))
	{
		Latitude = FMath::Fmod(FMath::Fmod(Latitude + 90.0f, 180.0f) + 180.0 , 180.0f) - 90.0f;
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherSettingsInfo, Longitude))
	{
		Longitude = FMath::Fmod(FMath::Fmod(Longitude + 180.0f, 360.0f) + 360.0 , 360.0f) - 180.0f;
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherSettingsInfo, PeriodOfDay))
	{
		
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherSettingsInfo, DaysOfMonth))
	{
		
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherSettingsInfo, NorthDirectionYawOffset))
	{
		NorthDirectionYawOffset =  FMath::Fmod(NorthDirectionYawOffset + 360.0f, 360.0f);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherSettingsInfo, InitTimeStampOfYear))
	{
		InitTimeStampOfYear = FMath::Max(InitTimeStampOfYear, 0.0f);
		InitTimeStampOfYear = FMath::Fmod(InitTimeStampOfYear, PeriodOfDay * DaysOfMonth * 12);
	}
}

bool AAetherSettingsInfo::CanEditChange(const FProperty* InProperty) const
{
	return Super::CanEditChange(InProperty);
}
#endif

void AAetherSettingsInfo::CaptureTimeStamp()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		InitTimeStampOfYear = Subsystem->GetSystemState().ProgressOfYear * (PeriodOfDay * DaysOfMonth * 12);
	}
}