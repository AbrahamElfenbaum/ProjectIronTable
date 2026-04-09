// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PanelLayoutSave.h"
#include "DraggablePanel.generated.h"

class UTextBlock;
class UCanvasPanelSlot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPanelStateChanged);

/**
 * A wrapper widget that makes any content widget draggable and resizable at runtime.
 *
 * Handles drag via a title bar zone at the top of the panel, and resize via a corner
 * zone at the bottom-right. Must be placed inside a UCanvasPanel for position and size
 * updates to take effect. Content is placed in the ContentSlot named slot.
 */
UCLASS()
class PROJECTIRONTABLE_API UDraggablePanel : public UUserWidget
{
	GENERATED_BODY()

public:

#pragma region Config
	/** Height in pixels of the drag zone at the top of the panel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TitleBarHeight;

	/** Minimum size the panel can be resized to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D MinSize;

	/** Default title displayed in the panel header. Set at runtime via SetPanelTitle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PanelTitle;

	/** Default position to reset to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D DefaultPosition;

	/** Default size to reset to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D DefaultSize;
#pragma endregion

private:

#pragma region Runtime State
	/** Unique identifier for this panel, used for saving and loading layout. Should be set in the editor or constructor. */
	FString PanelID;

	/** Offset from the panel's canvas position to the mouse at the start of a drag. */
	FVector2D DragOffset;

	/** Screen space mouse position at the start of a resize. */
	FVector2D ResizeStartMouse;

	/** Panel size at the start of a resize. */
	FVector2D ResizeStartSize;

	/** Cached canvas panel slot used to read and write position and size each frame. */
	UPROPERTY()
	TObjectPtr<UCanvasPanelSlot> CanvasSlot;
#pragma endregion

#pragma region Widget References
	/** Visual widget representing the drag handle area at the top of the panel. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> DragHandle;

	/** Named slot where the wrapped content widget is placed. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNamedSlot> ContentSlot;

	/** Visual widget representing the resize handle at the bottom-right corner. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> ResizeCorner;

	/** Text block displaying the panel title in the header. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;
#pragma endregion

protected:
	/** Caches the canvas slot and applies the default panel title. */
	virtual void NativeConstruct() override;

public:
	/** Sets the unique identifier used to save and restore this panel's layout. Should be called once after creation. */
	void SetPanelID(const FString& ID);

	/** Returns the unique identifier used to save and restore this panel's layout. */
	FString GetPanelID() const;

	/** Sets the panel title text at runtime. */
	void SetPanelTitle(FText NewTitle);

	/** Records the offset between the panel's canvas position and the mouse to anchor the drag correctly. */
	void StartDrag(FVector2D MousePos);

	/** Moves the panel to follow the mouse using the recorded drag offset. */
	void UpdateDrag(FVector2D MousePos);

	/** Called when the drag is released. Reserved for future cleanup. */
	void StopDrag();

	/** Records the mouse position and current panel size as the baseline for resize calculations. */
	void StartResize(FVector2D MousePos);

	/** Resizes the panel based on mouse delta from the resize start point, clamped to MinSize. */
	void UpdateResize(FVector2D MousePos);

	/** Called when the resize is released. Reserved for future cleanup. */
	void StopResize();

	/** Captures the current panel layout data, including position, size, and visibility, for saving. */
	FPanelLayoutData GetPanelLayoutData() const;

	/** Applies the given panel layout data to restore position, size, and visibility. */
	void ApplyPanelLayoutData(const FPanelLayoutData& LayoutData);

	/** Resets the panel to its default position and size. Does not affect visibility. */
	void ResetToDefaultLayout();

	/** Broadcast when the panel finishes being dragged or resized, signaling that the layout should be saved. */
	FOnPanelStateChanged OnPanelStateChanged;
};
