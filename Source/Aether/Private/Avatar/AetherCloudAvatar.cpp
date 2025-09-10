/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherCloudAvatar.h"

#include "Components/BillboardComponent.h"
#include "NiagaraComponent.h"

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
	
	RainFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RainFX"));
	RainFXComponent->SetupAttachment(RootComponent);
}

void AAetherCloudAvatar::BeginPlay()
{
	Super::BeginPlay();
}

void AAetherCloudAvatar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAetherCloudAvatar::UpdateFromSystemState(const FAetherState& State)
{
	
}