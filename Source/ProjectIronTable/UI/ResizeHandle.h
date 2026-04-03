// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResizeHandle.generated.h"

/**
 * Hit-testable widget placed in the bottom-right corner of a UDraggablePanel.
 * Captures mouse input on press and forwards resize events to the parent panel.
 */
UCLASS()
class PROJECTIRONTABLE_API UResizeHandle : public UUserWidget
{
	GENERATED_BODY()

private:
	/** True while the left mouse button is held down on this handle. */
	bool bActive;

	/** Activates resize mode, notifies the parent panel of the starting mouse position, and captures the mouse. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Forwards the current mouse position to the parent panel while resizing; passes through otherwise. */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Deactivates resize mode, notifies the parent panel, and releases mouse capture. */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

};
