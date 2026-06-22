// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "TilePlacementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "MapGrid.h"
#include "TileActor.h"

// Enables ticking so the ghost preview can ease toward its target rotation.
UTilePlacementComponent::UTilePlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Eases the ghost preview toward the target rotation each frame.
void UTilePlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsValid(GhostTileRef))
	{
		const FRotator NewRotation = FMath::RInterpTo(GhostTileRef->GetActorRotation(), CurrentRotation, DeltaTime, RotationInterpSpeed);
		GhostTileRef->SetActorRotation(NewRotation);
	}
}

// Caches the map grid and spawns the ghost preview tile.
void UTilePlacementComponent::Init(AMapGrid* MapGrid)
{
	WorldRef = GetWorld();

	if (!IsValid(WorldRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UTilePlacementComponent::Init — World is null"));
		return;
	}

	MapGridRef = MapGrid;

	if (!IsValid(MapGridRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UTilePlacementComponent::Init — Map Grid is null"));
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CurrentRotation = FRotator::ZeroRotator;

	GhostTileRef = WorldRef->SpawnActor<ATileActor>(TileClass, FVector::ZeroVector, CurrentRotation, SpawnParameters);

	if (!IsValid(GhostTileRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UTilePlacementComponent::Init — Ghost Tile is null"));
		return;
	}

	GhostTileRef->SetActorEnableCollision(false);
}

// Places a tile at the given grid cell, unless one already occupies it.
void UTilePlacementComponent::PlaceTile(FIntVector Cell)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (PlacedTiles.Contains(Cell)) return;

	ATileActor* Tile = WorldRef->SpawnActor<ATileActor>(
		TileClass, MapGridRef->GridToWorld(Cell), CurrentRotation, SpawnParameters);

	if (!IsValid(Tile))
	{
		UE_LOG(LogTemp, Warning, TEXT("UTilePlacementComponent::PlaceTile — Tile is null"));
		return;
	}

	PlacedTiles.Add(Cell, Tile);
}

// Deletes the tile occupying the given grid cell.
void UTilePlacementComponent::DeleteTile(FIntVector Cell)
{
	TObjectPtr<ATileActor>* Tile = PlacedTiles.Find(Cell);

	if (Tile && IsValid(*Tile))
	{
		(*Tile)->Destroy();
		PlacedTiles.Remove(Cell);
	}
}

// Adds a yaw step to the placement rotation, wrapped to [-180, 180].
void UTilePlacementComponent::RotateTile(float YawModifier)
{
	CurrentRotation.Yaw = FMath::UnwindDegrees(CurrentRotation.Yaw + YawModifier);
}

// Moves the ghost preview to the given grid cell.
void UTilePlacementComponent::UpdateGhostTile(FIntVector Cell)
{
	if (!IsValid(GhostTileRef)) return;
	GhostTileRef->SetActorLocation(MapGridRef->GridToWorld(Cell));
}
