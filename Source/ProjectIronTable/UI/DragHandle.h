// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DragHandle.generated.h"

/**
 * Hit-testable widget placed in the title bar of a UDraggablePanel.
 * Captures mouse input on press and forwards drag events to the parent panel.
 */
UCLASS()
class PROJECTIRONTABLE_API UDragHandle : public UUserWidget
{
	GENERATED_BODY()

private:
	/** True while the left mouse button is held down on this handle. */
	bool bActive;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
