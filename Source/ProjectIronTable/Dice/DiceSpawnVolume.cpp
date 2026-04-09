// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "DiceSpawnVolume.h"
#include "Components/BoxComponent.h"

// Creates the box component and sets it as the root.
ADiceSpawnVolume::ADiceSpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SetRootComponent(SpawnArea);
}

// Returns the world-space AABB of the spawn box component.
FBox ADiceSpawnVolume::GetSpawnBox() const
{
	return SpawnArea->Bounds.GetBox();
}

