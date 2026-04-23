// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ContextMenu.h"

#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"

#include "ContextMenuButton.h"
#include "MacroLibrary.h"

// Clears existing buttons and spawns a new button for each provided option.
void UContextMenu::SetMenuOptions(const TArray<FContextMenuOption>& Options)
{
	if (!ContextMenuButtonClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UContextMenu::SetMenuOptions — ContextMenuButtonClass is not set."));
		return;
	}

	MenuOptions.Empty();
	if (MenuOptionsBox)
	{
		MenuOptionsBox->ClearChildren();
	}

	for (const FContextMenuOption& Option : Options)
	{
		UContextMenuButton* MenuButton = CreateWidget<UContextMenuButton>(this, ContextMenuButtonClass);
		if (!IsValid(MenuButton))
		{
			UE_LOG(LogTemp, Warning, TEXT("UContextMenu::SetMenuOptions — Failed to create UContextMenuButton."));
			continue;
		}

		FContextMenuOption OptionLambda = Option;

		OptionLambda.OnClicked.BindLambda([this, Original = Option.OnClicked]()
			{
				CloseMenu();
				Original.ExecuteIfBound();
			});

		MenuButton->SetOption(OptionLambda);
		MenuOptions.Add(MenuButton);
		MenuOptionsBox->AddChild(MenuButton);
	}
}

// Offsets the content box within the overlay to place it at the given screen position.
void UContextMenu::SetMenuPosition(FVector2D Position)
{
	UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(ContextBox->Slot);
	CHECK_IF_VALID(OverlaySlot, );
	OverlaySlot->SetPadding(FMargin(Position.X, Position.Y, 0.f, 0.f));
	OverlaySlot->SetHorizontalAlignment(HAlign_Left);
	OverlaySlot->SetVerticalAlignment(VAlign_Top);
}

// Removes the menu widget from the viewport.
void UContextMenu::CloseMenu()
{
	RemoveFromParent();
}

// Closes the menu if the click is outside the content box; otherwise passes the event to children.
FReply UContextMenu::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FGeometry BoxGeometry = ContextBox->GetCachedGeometry();
	FVector2D AbsPos = BoxGeometry.GetAbsolutePosition();
	FVector2D AbsSize = BoxGeometry.GetAbsoluteSize();
	FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();

	bool bInsideBox = MousePos.X >= AbsPos.X && MousePos.X <= AbsPos.X + AbsSize.X
		&& MousePos.Y >= AbsPos.Y && MousePos.Y <= AbsPos.Y + AbsSize.Y;

	if (!bInsideBox)
	{
		CloseMenu();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}
