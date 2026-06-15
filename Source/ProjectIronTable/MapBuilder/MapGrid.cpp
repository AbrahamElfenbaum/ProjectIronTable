// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MapGrid.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"

// Creates the scene root component and disables ticking.
AMapGrid::AMapGrid()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	RootComponent = Root;
}

// Draws the debug grid visualization when play begins.
void AMapGrid::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	
	for (int32 i = 0; i < GridDimensions.Y + 1; i++)
	{
		FVector LocalStart = FVector(0.f, i * TileSize, 0.f);
		FVector WorldStart = GetActorTransform().TransformPosition(LocalStart);

		FVector LocalEnd = FVector(GridDimensions.X * TileSize, i * TileSize, 0.f);
		FVector WorldEnd = GetActorTransform().TransformPosition(LocalEnd);

		DrawDebugLine(World, WorldStart, WorldEnd, FColor::Green, true);
	}

	for (int32 i = 0; i < GridDimensions.X + 1; i++)
	{
		FVector LocalStart = FVector(i * TileSize, 0.f, 0.f);
		FVector WorldStart = GetActorTransform().TransformPosition(LocalStart);

		FVector LocalEnd = FVector(i * TileSize, GridDimensions.Y * TileSize, 0.f);
		FVector WorldEnd = GetActorTransform().TransformPosition(LocalEnd);

		DrawDebugLine(World, WorldStart, WorldEnd, FColor::Green, true);
	}

	for (int32 x = 0; x < GridDimensions.X; x++)
	{
		for (int32 y = 0; y < GridDimensions.Y; y++)
		{
			DrawDebugSphere(World, GridToWorld(FIntPoint(x, y)), 25.f, 12, FColor::Blue, true);
		}
	}
}

// Returns the world-space center of the given grid cell.
FVector AMapGrid::GridToWorld(FIntPoint Cell) const
{
	FVector LocalOffset = {Cell.X * TileSize + TileSize / 2,
						   Cell.Y * TileSize + TileSize / 2,
						   0};
	
	return GetActorTransform().TransformPosition(LocalOffset);
}

// Returns the grid cell that contains the given world location.
FIntPoint AMapGrid::WorldToGrid(FVector WorldLocation) const
{
	FVector LocalPosition = GetActorTransform().InverseTransformPosition(WorldLocation);
	FVector TileCount = LocalPosition / TileSize;
	return FIntPoint(FMath::FloorToInt(TileCount.X), FMath::FloorToInt(TileCount.Y));
}

