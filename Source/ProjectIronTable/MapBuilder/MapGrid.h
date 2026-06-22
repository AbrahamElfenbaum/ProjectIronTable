// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapGrid.generated.h"

class USceneComponent;

/** Defines the map builder grid area and converts between grid coordinates and world space. */
UCLASS()
class PROJECTIRONTABLE_API AMapGrid : public AActor
{
	GENERATED_BODY()
	
public:
	AMapGrid();

	/** Width and depth of a single tile in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Grid Settings")
	float TileSize = 100.f;

	/** Vertical distance between build levels in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Grid Settings")
	float TileHeight = 100.f;

	/** Number of tiles along the X and Y axes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Grid Settings")
	FIntPoint GridDimensions = FIntPoint(10, 10);

protected:

	/** Draws the debug grid visualization when play begins. */
	virtual void BeginPlay() override;

public:

	/** Returns the world-space position of the given grid cell: centered on X and Y, base-aligned on Z by level. */
	FVector GridToWorld(FIntVector Cell) const;

	/** Returns the grid cell that contains the given world location. */
	FIntVector WorldToGrid(FVector WorldLocation) const;

	/** Returns whether the given cell lies within the grid bounds. */
	bool IsValidCell(FIntVector Cell) const;
};
