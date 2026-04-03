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

// Stores the panel ID used to key into the layout save game.
void UDraggablePanel::SetPanelID(const FString& ID)
{
	PanelID = ID;
}

// Returns the panel ID used to key into the layout save game.
FString UDraggablePanel::GetPanelID() const
{
	return PanelID;
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
	OnPanelStateChanged.Broadcast();
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
	OnPanelStateChanged.Broadcast();
}

// Returns the panel's current position, size, and visibility as a layout data struct.
FPanelLayoutData UDraggablePanel::GetPanelLayoutData() const
{
	return FPanelLayoutData(
		CanvasSlot->GetPosition(), 
		CanvasSlot->GetSize(), 
		GetVisibility() == ESlateVisibility::Visible);
}

// Restores the panel's position, size, and visibility from the given layout data struct.
void UDraggablePanel::ApplyPanelLayoutData(const FPanelLayoutData& LayoutData)
{
	CanvasSlot->SetPosition(LayoutData.Position);
	CanvasSlot->SetSize(LayoutData.Size);
	SetVisibility(LayoutData.bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
