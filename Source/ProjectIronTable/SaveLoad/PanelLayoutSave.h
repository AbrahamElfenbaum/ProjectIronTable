// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PanelLayoutSave.generated.h"

/** Stores the saved position, size, and visibility of a single draggable panel. */
USTRUCT(BlueprintType)
struct FPanelLayoutData
{
	GENERATED_BODY()
	
	/** Canvas position of the panel in screen space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position;

	/** Width and height of the panel in pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Size;

	/** Whether the panel is visible or hidden. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisible;

	FPanelLayoutData():
		Position(FVector2D::Zero()),
		Size(FVector2D::Zero()),
		bVisible(true)
	{
	}

	FPanelLayoutData(FVector2D InPosition, FVector2D InSize, bool bInVisible):
		Position(InPosition),
		Size(InSize),
		bVisible(bInVisible)
	{
	}

};

/**
 * Saves and restores the layout of all draggable panels, including position, size, and visibility.
 * One instance is stored per client under the save slot "PanelLayout".
 */
UCLASS()
class PROJECTIRONTABLE_API UPanelLayoutSave : public USaveGame
{
	GENERATED_BODY()

public:

	/** Save slot name used for all panel layout load and save calls. */
	static const FString SaveSlotName;

	/** Map of panel names to their layout data, including position, size, and visibility. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FPanelLayoutData> PanelLayouts;
};
