/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherLightingAvatar.h"

#include "Components/BillboardComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"

#include "AetherWorldSubsystem.h"

AAetherLightingAvatar::AAetherLightingAvatar()
{
	PrimaryActorTick.bCanEverTick = false;
	
#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif
	
	USceneComponent* AvatarRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AvatarRoot"));
	RootComponent = AvatarRootComponent;
	
	SkyDomeComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkyDomeMesh"));
	SkyDomeComponent->SetupAttachment(RootComponent);
	SkyDomeComponent->SetCollisionProfileName(FName("NoCollision"));
	SkyDomeComponent->SetGenerateOverlapEvents(false);
	
	SunLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLightComponent->SetupAttachment(RootComponent);
	SunLightComponent->SetRelativeLocation(FVector(0.0f, -200.0f, 300.0f));
	SunLightComponent->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
	
	MoonLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
	MoonLightComponent->SetupAttachment(RootComponent);
	MoonLightComponent->bAffectsWorld = false;
	MoonLightComponent->SetRelativeLocation(FVector(0.0f, 200.0f, 200.0f));
	
	SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLightComponent->SetupAttachment(RootComponent);
	SkyLightComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	
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
}

void AAetherLightingAvatar::BeginPlay()
{
	Super::BeginPlay();
	
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
    {
    	Subsystem->RegisterLightingAvatar(this);
    }
}

void AAetherLightingAvatar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAetherLightingAvatar::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
			{
				Subsystem->RegisterLightingAvatar(this);
			}
		}
	}
#endif
}

void AAetherLightingAvatar::UpdateLighting(const FAetherState& State)
{
	SunLightComponent->SetWorldRotation(State.SunLightDirection.Rotation());
}