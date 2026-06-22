// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TilePlacementComponent.generated.h"

class ATileActor;
class AMapGrid;

/** Owns map-builder tile placement: spawns and deletes tiles by grid cell, tracks cell occupancy, and drives the ghost preview. */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API UTilePlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Enables ticking so the ghost preview can ease toward its target rotation. */
	UTilePlacementComponent();

private:

#pragma region Runtime References
	/** Cached world reference, used to spawn tiles and the ghost preview. */
	UPROPERTY()
	TObjectPtr<UWorld> WorldRef;

	/** Cached reference to the active map grid in the level. */
	UPROPERTY()
	TObjectPtr<AMapGrid> MapGridRef;

	/** Preview tile that follows the cursor to show the current snap location. */
	UPROPERTY()
	TObjectPtr<ATileActor> GhostTileRef;

	/** Maps each occupied grid cell to the tile placed there; used to prevent stacking and to delete by cell. */
	UPROPERTY()
	TMap<FIntVector, TObjectPtr<ATileActor>> PlacedTiles;

	/** Rotation applied to the ghost preview and to newly placed tiles. */
	UPROPERTY()
	FRotator CurrentRotation;
#pragma endregion

public:

#pragma region Config
	/** Tile class to spawn when placing on the grid. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Tile")
	TSubclassOf<ATileActor> TileClass;

	/** Speed at which the ghost preview eases toward the target rotation. */
	UPROPERTY(EditAnywhere, Category = "Map Tile")
	float RotationInterpSpeed = 8.0f;
#pragma endregion

protected:

	/** Eases the ghost preview toward the target rotation each frame. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	/** Caches the map grid and spawns the ghost preview tile. */
	void Init(AMapGrid* MapGrid);

	/** Places a tile at the given grid cell, unless one already occupies it. */
	void PlaceTile(FIntVector Cell);

	/** Deletes the tile occupying the given grid cell. */
	void DeleteTile(FIntVector Cell);

	/** Adds a yaw step to the placement rotation, wrapped to [-180, 180]. */
	void RotateTile(float YawModifier);

	/** Moves the ghost preview to the given grid cell. */
	void UpdateGhostTile(FIntVector Cell);
};
