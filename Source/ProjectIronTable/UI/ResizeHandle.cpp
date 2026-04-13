// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ResizeHandle.h"
#include "DraggablePanel.h"

// Activates resize mode, notifies the parent panel of the starting mouse position, and captures the mouse.
FReply UResizeHandle::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UDraggablePanel* Panel = GetTypedOuter<UDraggablePanel>();
	if (!Panel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UResizeHandle::NativeOnMouseButtonDown — outer UDraggablePanel not found."));
		return FReply::Unhandled();
	}
	bActive = true;
	Panel->StartResize(InMouseEvent.GetScreenSpacePosition());
	return FReply::Handled().CaptureMouse(TakeWidget());
}

// Forwards the current mouse position to the parent panel while resizing; passes through otherwise.
FReply UResizeHandle::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bActive)
	{
		UDraggablePanel* Panel = GetTypedOuter<UDraggablePanel>();
		if (!Panel)
		{
			UE_LOG(LogTemp, Warning, TEXT("UResizeHandle::NativeOnMouseMove — outer UDraggablePanel not found."));
			return FReply::Unhandled();
		}
		Panel->UpdateResize(InMouseEvent.GetScreenSpacePosition());
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
	UDraggablePanel* Panel = GetTypedOuter<UDraggablePanel>();
	if (!Panel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UResizeHandle::NativeOnMouseButtonUp — outer UDraggablePanel not found."));
		return FReply::Unhandled();
	}
	bActive = false;
	Panel->StopResize();
	return FReply::Handled().ReleaseMouseCapture();
}
