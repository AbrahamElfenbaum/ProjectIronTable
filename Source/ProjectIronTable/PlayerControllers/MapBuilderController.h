// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseCameraController.h"
#include "MapBuilderController.generated.h"

class AMapGrid;
class ATileActor;
class UTilePlacementComponent;

/** Player controller for the map builder. Inherits free-look camera and adds grid-snapped tile placement with a ghost preview. */
UCLASS()
class PROJECTIRONTABLE_API AMapBuilderController : public ABaseCameraController
{
	GENERATED_BODY()

public:

	/** Creates the tile placement component. */
	AMapBuilderController();

protected:

#pragma region Runtime References
	/** Cached reference to the active map grid, used for cursor-to-cell conversion. */
	UPROPERTY()
	TObjectPtr<AMapGrid> MapGridRef;
#pragma endregion

public:

#pragma region Components
	/** Owns tile placement, deletion, the occupancy map, and the ghost preview. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTilePlacementComponent> TilePlacementComponent;
#pragma endregion

#pragma region Build Input
	/** Input mapping context active while in build mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputMappingContext> IMC_Build;

	/** Raises or lowers the build level the cursor places tiles on. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputAction> IA_ChangeLevel;

	/** Deletes the placed tile under the cursor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputAction> IA_DeleteTile;

	/** Places a tile on the grid cell under the cursor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputAction> IA_PlaceTile;

	/** Rotates the ghost preview and future placements in 90-degree steps. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputAction> IA_RotateTile;

	/** Toggles the locked state of the tile under the cursor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputAction> IA_ToggleTileLock;
#pragma endregion

private:

#pragma region State
	/** Grid level the cursor currently builds on; placement and the ghost ride this level's plane. */
	int32 ActiveBuildLevel = 0;
#pragma endregion

#pragma region Input Handlers
	/** Raises or lowers the active build level. */
	void Input_ChangeLevel(const FInputActionValue& Value);
	
	/** Destroys the placed tile under the cursor. */
	void Input_DeleteTile(const FInputActionValue& Value);

	/** Spawns a tile at the grid cell under the cursor. */
	void Input_PlaceTile(const FInputActionValue& Value);

	/** Adds a 90-degree step to the current placement rotation. */
	void Input_RotateTile(const FInputActionValue& Value);

	/** Toggles the locked state of the tile under the cursor. */
	void Input_ToggleTileLock(const FInputActionValue& Value);
#pragma endregion

#pragma region Private Functions
	/** Returns the grid cell under the mouse cursor; false if the cursor ray misses the grid plane. */
	bool GetGridCellUnderCursor(FIntVector& OutCell) const;

	/** Returns the tile under the cursor, or nullptr if the cursor ray hits no tile. */
	ATileActor* FindTileActor();
#pragma endregion

protected:

	/** Adds the build input context and binds the rotate, place, and delete actions, after the base camera setup. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Caches the map grid used for cursor-to-cell conversion, after the base camera setup. */
	virtual void BeginPlay() override;

	/** Updates the ghost tile to follow the grid cell under the cursor each frame. */
	virtual void PlayerTick(float DeltaTime) override;
};
