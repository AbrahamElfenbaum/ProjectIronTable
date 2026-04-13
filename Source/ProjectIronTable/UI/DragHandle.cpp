// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "DragHandle.h"
#include "DraggablePanel.h"

// Activates drag mode, notifies the parent panel of the starting mouse position, and captures the mouse.
FReply UDragHandle::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UDraggablePanel* Panel = GetTypedOuter<UDraggablePanel>();
	if (!Panel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDragHandle::NativeOnMouseButtonDown — outer UDraggablePanel not found."));
		return FReply::Unhandled();
	}
	bActive = true;
	Panel->StartDrag(InMouseEvent.GetScreenSpacePosition());
	return FReply::Handled().CaptureMouse(TakeWidget());
}

// Forwards the current mouse position to the parent panel while dragging; passes through otherwise.
FReply UDragHandle::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bActive)
	{
		UDraggablePanel* Panel = GetTypedOuter<UDraggablePanel>();
		if (!Panel)
		{
			UE_LOG(LogTemp, Warning, TEXT("UDragHandle::NativeOnMouseMove — outer UDraggablePanel not found."));
			return FReply::Unhandled();
		}
		Panel->UpdateDrag(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	else
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}
}

// Deactivates drag mode, notifies the parent panel, and releases mouse capture.
FReply UDragHandle::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UDraggablePanel* Panel = GetTypedOuter<UDraggablePanel>();
	if (!Panel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDragHandle::NativeOnMouseButtonUp — outer UDraggablePanel not found."));
		return FReply::Unhandled();
	}
	bActive = false;
	Panel->StopDrag();
	return FReply::Handled().ReleaseMouseCapture();
}
