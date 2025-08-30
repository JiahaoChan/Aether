/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherCloudAvatar.h"

#include "Components/BillboardComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"

#include "AetherWorldSubsystem.h"

AAetherCloudAvatar::AAetherCloudAvatar()
{
	PrimaryActorTick.bCanEverTick = false;
	
#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif
	
	USceneComponent* AvatarRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AvatarRoot"));
	RootComponent = AvatarRootComponent;
	
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

void AAetherCloudAvatar::BeginPlay()
{
	Super::BeginPlay();
	
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
    {
    	Subsystem->RegisterCloudAvatar(this);
    }
}

void AAetherCloudAvatar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAetherCloudAvatar::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
			{
				Subsystem->RegisterCloudAvatar(this);
			}
		}
	}
#endif
}

void AAetherCloudAvatar::Destroyed()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
    {
    	Subsystem->UnregisterCloudAvatar(this);
    }
	
	Super::Destroyed();
}

void AAetherCloudAvatar::UpdateCloudLayers(const FAetherState& State)
{
	
}