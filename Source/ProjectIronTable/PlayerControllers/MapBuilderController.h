// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseCameraController.h"
#include "MapBuilderController.generated.h"

class AMapGrid;
class UTilePlacementComponent;

/** Player controller for the map builder. Inherits free-look camera and adds grid-snapped tile placement with a ghost preview. */
UCLASS()
class PROJECTIRONTABLE_API AMapBuilderController : public ABaseCameraController
{
	GENERATED_BODY()

protected:

#pragma region Runtime References
	/** Cached reference to the active map grid, used for cursor-to-cell conversion. */
	UPROPERTY()
	TObjectPtr<AMapGrid> MapGridRef;
#pragma endregion

public:

	/** Creates the tile placement component. */
	AMapBuilderController();

#pragma region Components
	/** Owns tile placement, deletion, the occupancy map, and the ghost preview. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTilePlacementComponent> TilePlacementComponent;
#pragma endregion

#pragma region Build Input
	/** Input mapping context active while in build mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputMappingContext> IMC_Build;

	/** Rotates the ghost preview and future placements in 90-degree steps. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputAction> IA_RotateTile;

	/** Places a tile on the grid cell under the cursor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputAction> IA_PlaceTile;

	/** Deletes the placed tile under the cursor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputAction> IA_DeleteTile;
#pragma endregion

private:

#pragma region Input Handlers
	/** Adds a 90-degree step to the current placement rotation. */
	void Input_RotateTile(const FInputActionValue& Value);

	/** Spawns a tile at the grid cell under the cursor. */
	void Input_PlaceTile(const FInputActionValue& Value);

	/** Destroys the placed tile under the cursor. */
	void Input_DeleteTile(const FInputActionValue& Value);
#pragma endregion

#pragma region Private Functions
	/** Returns the grid cell under the mouse cursor; false if the cursor ray misses the grid plane. */
	bool GetGridCellUnderCursor(FIntPoint& OutCell) const;
#pragma endregion

protected:

	/** Adds the build input context and binds the rotate, place, and delete actions, after the base camera setup. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Caches the map grid used for cursor-to-cell conversion, after the base camera setup. */
	virtual void BeginPlay() override;

	/** Updates the ghost tile to follow the grid cell under the cursor each frame. */
	virtual void PlayerTick(float DeltaTime) override;
};
