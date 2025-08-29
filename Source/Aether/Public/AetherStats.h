/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		    Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"

DECLARE_STATS_GROUP(TEXT("AetherTickGroup"), STATGROUP_Aether, STATCAT_Advanced)

DECLARE_CYCLE_STAT(TEXT("AetherWorldSubsystem_Tick"), STAT_AetherWorldSubsystem_Tick, STATGROUP_Aether);
DECLARE_CYCLE_STAT(TEXT("AetherController_Tick"), STAT_AetherController_Tick, STATGROUP_Aether);