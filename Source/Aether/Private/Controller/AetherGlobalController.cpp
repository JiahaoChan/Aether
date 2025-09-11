/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherGlobalController.h"

#include "Components/BillboardComponent.h"

#include "AetherWorldSubsystem.h"

AAetherGlobalController::AAetherGlobalController()
{
#if WITH_EDITORONLY_DATA
	UBillboardComponent* SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	
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
	
#if WITH_EDITORONLY_DATA
	VisualizeComponent = CreateDefaultSubobject<UAetherGlobalControllerVisualizeComponent>(TEXT("VisualizeComponent"));
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

#if WITH_EDITOR
void AAetherGlobalController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
	const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;
	
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherGlobalController, Latitude))
	{
		Latitude = FMath::Fmod(FMath::Fmod(Latitude + 90.0f, 180.0f) + 180.0 , 180.0f) - 90.0f;
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherGlobalController, Longitude))
	{
		Longitude = FMath::Fmod(FMath::Fmod(Longitude + 180.0f, 360.0f) + 360.0 , 360.0f) - 180.0f;
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherGlobalController, PeriodOfDay))
	{
		
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherGlobalController, DaysOfMonth))
	{
		
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherGlobalController, NorthDirectionYawOffset))
	{
		NorthDirectionYawOffset =  FMath::Fmod(NorthDirectionYawOffset + 360.0f, 360.0f);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AAetherGlobalController, InitTimeStampOfYear))
	{
		InitTimeStampOfYear = FMath::Max(InitTimeStampOfYear, 0.0f);
		InitTimeStampOfYear = FMath::Fmod(InitTimeStampOfYear, PeriodOfDay * DaysOfMonth * 12);
	}
}

bool AAetherGlobalController::CanEditChange(const FProperty* InProperty) const
{
	return Super::CanEditChange(InProperty);
}
#endif

void AAetherGlobalController::CaptureTimeStamp()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		InitTimeStampOfYear = Subsystem->GetSystemState().ProgressOfYear * (PeriodOfDay * DaysOfMonth * 12);
	}
}