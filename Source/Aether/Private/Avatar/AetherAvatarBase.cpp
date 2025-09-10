/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#include "AetherAvatarBase.h"

#include "AetherWorldSubsystem.h"

AAetherAvatarBase::AAetherAvatarBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAetherAvatarBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		Subsystem->RegisterAvatar(this);
	}
}

void AAetherAvatarBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAetherAvatarBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR
	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Type::Editor)
		{
			if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
			{
				Subsystem->RegisterAvatar(this);
			}
		}
	}
#endif
}

void AAetherAvatarBase::Destroyed()
{
	if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(this))
	{
		Subsystem->UnregisterAvatar(this);
	}
	
	Super::Destroyed();
}