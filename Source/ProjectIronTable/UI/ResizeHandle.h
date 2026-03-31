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

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
};
