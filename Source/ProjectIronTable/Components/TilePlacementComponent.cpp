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

// Caches the map grid and spawns the ghost preview tile.
void UTilePlacementComponent::BeginPlay()
{
	Super::BeginPlay();

	WorldRef = GetWorld();

	if (!IsValid(WorldRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UTilePlacementComponent::BeginPlay - World is null"));
		return;
	}

	AActor* Found = UGameplayStatics::GetActorOfClass(this, AMapGrid::StaticClass());
	MapGridRef = Cast<AMapGrid>(Found);

	if (!IsValid(MapGridRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UTilePlacementComponent::BeginPlay - Map Grid is null"));
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CurrentRotation = FRotator::ZeroRotator;

	GhostTileRef = WorldRef->SpawnActor<ATileActor>(TileClass, FVector::ZeroVector, CurrentRotation, SpawnParameters);

	if (!IsValid(GhostTileRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UTilePlacementComponent::BeginPlay - Ghost Tile is null"));
		return;
	}

	GhostTileRef->SetActorEnableCollision(false);
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

// Places a tile at the given grid cell, unless one already occupies it.
void UTilePlacementComponent::PlaceTile(FIntPoint Cell)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (PlacedTiles.Contains(Cell)) return;

	ATileActor* Tile = WorldRef->SpawnActor<ATileActor>(
		TileClass, MapGridRef->GridToWorld(Cell), CurrentRotation, SpawnParameters);

	if (!IsValid(Tile))
	{
		UE_LOG(LogTemp, Warning, TEXT("UTilePlacementComponent::PlaceTile - Tile is null"));
		return;
	}

	PlacedTiles.Add(Cell, Tile);
}

// Deletes the tile occupying the given grid cell.
void UTilePlacementComponent::DeleteTile(FIntPoint Cell)
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
void UTilePlacementComponent::UpdateGhostTile(FIntPoint Cell)
{
	if (!IsValid(GhostTileRef)) return;
	GhostTileRef->SetActorLocation(MapGridRef->GridToWorld(Cell));
}

