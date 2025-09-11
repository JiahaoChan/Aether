/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherControllerBase.h"

#include "AetherWorldSubsystem.h"

AAetherControllerBase::AAetherControllerBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bEnableAutoLODGeneration = false;
#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif
	
	USceneComponent* ControllerRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ControllerRoot"));
	RootComponent = ControllerRootComponent;
}

void AAetherControllerBase::BeginPlay()
{
	Super::BeginPlay();
}

void AAetherControllerBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
            {
				InitializeController();
				Subsystem->RegisterController(this);
            }
		}
	}
#endif
}

void AAetherControllerBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		InitializeController();
		Subsystem->RegisterController(this);
	}
}

void AAetherControllerBase::Destroyed()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		Subsystem->UnregisterController(this);
	}
	Super::Destroyed();
}