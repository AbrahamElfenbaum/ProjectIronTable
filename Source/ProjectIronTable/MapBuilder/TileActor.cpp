// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "TileActor.h"
#include "Components/StaticMeshComponent.h"

// Creates the tile's static mesh component and sets it as the root.
ATileActor::ATileActor()
{
	PrimaryActorTick.bCanEverTick = false;
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tile Mesh"));
	SetRootComponent(TileMesh);
	TileMesh->SetMobility(EComponentMobility::Movable);
}
