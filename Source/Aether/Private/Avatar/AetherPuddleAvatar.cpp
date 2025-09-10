/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherPuddleAvatar.h"

#include "Components/BillboardComponent.h"
#include "Components/DecalComponent.h"

#include "AetherWorldSubsystem.h"

int32 GAetherFeaturePuddle = 1;
FAutoConsoleVariableRef CVarAetherFeaturePuddle(
	TEXT("at.Feature.Puddle"),
	GAetherFeaturePuddle,
	TEXT("0 corresponds to enable the feature of puddle avatar, 1 corresponds to disable the feature of puddle avatar."),
	ECVF_Scalability
	);

/*
void DetailModeSink()
{
	//This Cvar sink can happen before the one which primes the cached scalability cvars so we must grab this ourselves.
	IConsoleManager& ConsoleMan = IConsoleManager::Get();
	static const auto DetailMode = ConsoleMan.FindTConsoleVariableDataInt(TEXT("at.Feature.Puddle"));
	int32 NewDetailMode = DetailMode->GetValueOnGameThread();
	static int32 CachedDetailMode = NewDetailMode;
	
	if (CachedDetailMode != NewDetailMode)
	{
		CachedDetailMode = NewDetailMode;
		
		for (TObjectIterator<AAetherPuddleAvatar> It; It; ++It)
		{
			//We must also reset on next tick rather than immediately as the cached cvar values are read internally to determin detail mode.
			//It->ResetNextTick();
		}
	}
}

static FAutoConsoleVariableSink CVarDetailModeSink(FConsoleCommandDelegate::CreateStatic(&DetailModeSink));
*/

AAetherPuddleAvatarBase::AAetherPuddleAvatarBase()
{
	USceneComponent* AvatarRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AvatarRoot"));
	RootComponent = AvatarRootComponent;
}

void AAetherPuddleAvatarBase::BeginPlay()
{
	if (GAetherFeaturePuddle > 0)
	{
		if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
		{
			Subsystem->RegisterAvatar(this);
		}
	}
	AActor::BeginPlay();
}

AAetherPuddleAvatar_Plane::AAetherPuddleAvatar_Plane()
{
	WaterSurfaceMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterSurfaceMesh"));
	WaterSurfaceMeshComponent->SetMobility(EComponentMobility::Type::Movable);
	WaterSurfaceMeshComponent->SetCollisionProfileName(FName("OverlayAll"));
	WaterSurfaceMeshComponent->SetupAttachment(RootComponent);
	
	ConstantHeight = 0.0f;
	MaxHeight = 15.0f;
	
#if WITH_EDITORONLY_DATA
	UBillboardComponent* SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	
	if (!IsRunningCommandlet())
	{
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

void AAetherPuddleAvatar_Plane::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	WaterSurfaceMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, ConstantHeight));
}

void AAetherPuddleAvatar_Plane::UpdateFromSystemState(const FAetherState& State)
{
	FVector LocalLocation = WaterSurfaceMeshComponent->GetRelativeLocation();
	float NewZ = State.PuddleRainRemain * MaxHeight + ConstantHeight;
	if (NewZ != LocalLocation.Z)
	{
		LocalLocation.Z = NewZ;
        WaterSurfaceMeshComponent->SetRelativeLocation(LocalLocation);
        bool bNewVisible = NewZ > UE_KINDA_SMALL_NUMBER;
        if (bNewVisible != WaterSurfaceMeshComponent->GetVisibleFlag())
        {
        	WaterSurfaceMeshComponent->SetVisibility(bNewVisible);
        }
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AAetherPuddleAvatar_Decal::AAetherPuddleAvatar_Decal()
{
	WaterSurfaceDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("WaterSurfaceDecal"));
	WaterSurfaceDecalComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	WaterSurfaceDecalComponent->SetupAttachment(RootComponent);
	
#if WITH_EDITORONLY_DATA
	UBillboardComponent * SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	
	if (!IsRunningCommandlet())
	{
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