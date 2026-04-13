// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ContextMenu.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"

#include "ContextMenuButton.h"

FReply UContextMenu::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	CloseMenu();
	return FReply::Handled();
}

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

void UContextMenu::CloseMenu()
{
	RemoveFromParent();
}
