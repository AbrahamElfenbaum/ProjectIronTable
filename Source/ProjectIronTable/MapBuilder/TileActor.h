// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileActor.generated.h"

class UStaticMeshComponent;

/** A single tile placed on the map grid, represented by a static mesh. */
UCLASS()
class PROJECTIRONTABLE_API ATileActor : public AActor
{
	GENERATED_BODY()

public:

#pragma region Components
	/** Static mesh that visually represents this tile. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	TObjectPtr<UStaticMeshComponent> TileMesh;
#pragma endregion

	/** Creates the tile's static mesh component and sets it as the root. */
	ATileActor();
};
