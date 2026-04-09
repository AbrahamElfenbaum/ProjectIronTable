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
	GameTypeLabel->SetText(FText::FromString(Label));
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
	return GameTypeLabel->GetText().ToString();
}

// Enables or disables the button to reflect whether campaigns exist for this game type.
void UGameTypeButton::SetInteractable(bool bInteractable)
{
	GameTypeTab->SetIsEnabled(bInteractable);
}

// Applies SelectedTabColor or UnselectedTabColor to the button background based on bSelected.
void UGameTypeButton::SetSelected(bool bSelected)
{
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
	OnGameTypeSelected.Broadcast(GameTypeLabel->GetText().ToString());
}
