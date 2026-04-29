// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DiceSpawnVolume.generated.h"

class UBoxComponent;

/** An actor that defines the box region in which dice are spawned. */
UCLASS()
class PROJECTIRONTABLE_API ADiceSpawnVolume : public AActor
{
	GENERATED_BODY()

public:

#pragma region Components
	/** Box component that defines the spawn region boundaries. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	TObjectPtr<UBoxComponent> SpawnArea;
#pragma endregion

#pragma region Public Methods
	/** Returns the world-space axis-aligned bounding box of the spawn area. */
	FBox GetSpawnBox() const;
#pragma endregion

	/** Creates the box component and sets it as the root. */
	ADiceSpawnVolume();
};
