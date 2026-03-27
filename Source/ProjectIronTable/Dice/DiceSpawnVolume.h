// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "DiceSpawnVolume.generated.h"

/** An actor that defines the box region in which dice are spawned. */
UCLASS()
class PROJECTIRONTABLE_API ADiceSpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	ADiceSpawnVolume();

	// -- Components --

	/** Box component that defines the spawn region boundaries. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	TObjectPtr<UBoxComponent> SpawnArea;

public:

	/** Returns the world-space axis-aligned bounding box of the spawn area. */
	FBox GetSpawnBox() const;
};
