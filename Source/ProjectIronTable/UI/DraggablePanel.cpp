// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "DraggablePanel.h"
#include "Components/CanvasPanelSlot.h"

// Caches the canvas slot and applies the default panel title.
void UDraggablePanel::NativeConstruct()
{
	Super::NativeConstruct();
	CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (TitleText)
	{
		TitleText->SetText(PanelTitle);
	}
}

// Updates the stored title and refreshes the title text block.
void UDraggablePanel::SetPanelTitle(FText NewTitle)
{
	if (TitleText)
	{
		PanelTitle = NewTitle;
		TitleText->SetText(NewTitle);
	}
}

// Records the offset between the panel's canvas position and the mouse to anchor the drag correctly.
void UDraggablePanel::StartDrag(FVector2D MousePos)
{
	DragOffset = CanvasSlot->GetPosition() - MousePos;
}

// Moves the panel to follow the mouse using the recorded drag offset.
void UDraggablePanel::UpdateDrag(FVector2D MousePos)
{
	CanvasSlot->SetPosition(MousePos + DragOffset);
}

// Called when the drag is released. Reserved for future cleanup.
void UDraggablePanel::StopDrag()
{
}

// Records the mouse position and current panel size as the baseline for resize calculations.
void UDraggablePanel::StartResize(FVector2D MousePos)
{
	ResizeStartMouse = MousePos;
	ResizeStartSize = CanvasSlot->GetSize();
}

// Resizes the panel based on mouse delta from the resize start point, clamped to MinSize.
void UDraggablePanel::UpdateResize(FVector2D MousePos)
{
	FVector2D Delta = MousePos - ResizeStartMouse;
	CanvasSlot->SetSize((ResizeStartSize + Delta).ComponentMax(MinSize));
}

// Called when the resize is released. Reserved for future cleanup.
void UDraggablePanel::StopResize()
{
}
