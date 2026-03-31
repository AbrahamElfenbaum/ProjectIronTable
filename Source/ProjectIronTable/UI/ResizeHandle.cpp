// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ResizeHandle.h"
#include "DraggablePanel.h"

// Activates resize mode, notifies the parent panel of the starting mouse position, and captures the mouse.
FReply UResizeHandle::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bActive = true;
	GetTypedOuter<UDraggablePanel>()->StartResize(InMouseEvent.GetScreenSpacePosition());
	return FReply::Handled().CaptureMouse(TakeWidget());
}

// Forwards the current mouse position to the parent panel while resizing; passes through otherwise.
FReply UResizeHandle::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bActive)
	{
		GetTypedOuter<UDraggablePanel>()->UpdateResize(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	else
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}
}

// Deactivates resize mode, notifies the parent panel, and releases mouse capture.
FReply UResizeHandle::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bActive = false;
	GetTypedOuter<UDraggablePanel>()->StopResize();
	return FReply::Handled().ReleaseMouseCapture();
}
