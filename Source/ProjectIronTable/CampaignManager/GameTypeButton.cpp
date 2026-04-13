// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "GameTypeButton.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

// Binds the tab button click delegate.
void UGameTypeButton::NativeConstruct()
{
	Super::NativeConstruct();
	if (GameTypeTab)
	{
		GameTypeTab->OnClicked.AddDynamic(this, &UGameTypeButton::OnGameTypeButtonClicked);
	}
}

// Sets the game type label text.
void UGameTypeButton::SetLabel(const FString& Label)
{
	if (GameTypeLabel)
	{
		GameTypeLabel->SetText(FText::FromString(Label));
	}
}

// Stores the selected and unselected background colors for use by SetSelected.
void UGameTypeButton::SetTabColors(const FLinearColor& InSelectedTabColor, const FLinearColor& InUnselectedTabColor)
{
	SelectedTabColor = InSelectedTabColor;
	UnselectedTabColor = InUnselectedTabColor;
}

// Returns the current label text as a string.
FString UGameTypeButton::GetLabel() const
{
	if (!GameTypeLabel)
	{
		return FString();
	}
	return GameTypeLabel->GetText().ToString();
}

// Enables or disables the button to reflect whether campaigns exist for this game type.
void UGameTypeButton::SetInteractable(bool bInteractable)
{
	if (GameTypeTab)
	{
		GameTypeTab->SetIsEnabled(bInteractable);
	}
}

// Applies SelectedTabColor or UnselectedTabColor to the button background based on bSelected.
void UGameTypeButton::SetSelected(bool bSelected)
{
	if (!GameTypeTab)
	{
		return;
	}

	if (bSelected)
	{
		GameTypeTab->SetBackgroundColor(SelectedTabColor);
	}
	else
	{
		GameTypeTab->SetBackgroundColor(UnselectedTabColor);
	}
}

// Broadcasts OnGameTypeSelected with the current label text as the game type name.
void UGameTypeButton::OnGameTypeButtonClicked()
{
	if (GameTypeLabel)
	{
		OnGameTypeSelected.Broadcast(GameTypeLabel->GetText().ToString());
	}
}
