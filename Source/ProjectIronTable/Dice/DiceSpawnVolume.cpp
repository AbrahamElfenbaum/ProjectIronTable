// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "DiceSpawnVolume.h"

ADiceSpawnVolume::ADiceSpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SetRootComponent(SpawnArea);
}

FBox ADiceSpawnVolume::GetSpawnBox() const
{
	return SpawnArea->Bounds.GetBox();
}

