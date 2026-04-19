// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ContextMenuButton.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

// Assigns the option, updates the label text, and binds the click handler.
void UContextMenuButton::SetOption(const FContextMenuOption& InOption)
{
	Option = InOption;
	ButtonLabel->SetText(FText::FromString(Option.ButtonName));
	MenuButton->OnClicked.AddDynamic(this, &UContextMenuButton::OnMenuButtonClicked);
}

// Executes the option's OnClicked delegate when the button is pressed.
void UContextMenuButton::OnMenuButtonClicked()
{
	Option.OnClicked.ExecuteIfBound();
}
