// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseCameraController.h"
#include "MapBuilderController.generated.h"

class ATileActor;
class AMapGrid;

/** Player controller for the map builder. Inherits free-look camera and adds grid-snapped tile placement with a ghost preview. */
UCLASS()
class PROJECTIRONTABLE_API AMapBuilderController : public ABaseCameraController
{
	GENERATED_BODY()

protected:

#pragma region Runtime References
	/** Cached reference to the active map grid in the level. */
	UPROPERTY()
	TObjectPtr<AMapGrid> MapGridRef;

	/** Preview tile that follows the cursor to show the current snap location. */
	UPROPERTY()
	TObjectPtr<ATileActor> GhostTileRef;
#pragma endregion

public:

#pragma region Config
	/** Tile to spawn when placing on the grid. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map Tile")
	TObjectPtr<ATileActor> TileActor;
#pragma endregion

#pragma region Build Input
	/** Input mapping context active while in build mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputMappingContext> IMC_Build;

	/** Places a tile on the grid cell under the cursor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build Input")
	TObjectPtr<UInputAction> IA_PlaceTile;
#pragma endregion

private:

#pragma region Input Handlers
	/** Spawns a tile at the grid cell under the cursor. */
	void Input_PlaceTile(const FInputActionValue& Value);
#pragma endregion

#pragma region Private Functions
	/** Returns the grid cell under the mouse cursor; false if the cursor ray misses the grid plane. */
	bool GetGridCellUnderCursor(FIntPoint& OutCell) const;
#pragma endregion

protected:

	/** Adds the build input context and binds the place-tile action, after the base camera setup. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Caches the map grid and spawns the ghost preview tile, after the base camera setup. */
	virtual void BeginPlay() override;

	/** Updates the ghost tile to follow the grid cell under the cursor each frame. */
	virtual void PlayerTick(float DeltaTime) override;
};
