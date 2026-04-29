// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "GameTypeTab.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

// Broadcasts OnGameTypeSelected with the current label text as the game type name.
void UGameTypeTab::OnGameTypeTabClicked()
{
	OnGameTypeSelected.Broadcast(GetLabel());
}

// Binds the tab button click delegate.
void UGameTypeTab::NativeConstruct()
{
	Super::NativeConstruct();
	if (GameTypeTab)
	{
		GameTypeTab->OnClicked.AddDynamic(this, &UGameTypeTab::OnGameTypeTabClicked);
	}
}

// Sets the game type label text.
void UGameTypeTab::SetLabel(const FString& Label)
{
	if (GameTypeLabel)
	{
		GameTypeLabel->SetText(FText::FromString(Label));
	}
}

// Stores the selected and unselected background colors for use by SetSelected.
void UGameTypeTab::SetTabColors(const FLinearColor& InSelectedTabColor, const FLinearColor& InUnselectedTabColor)
{
	SelectedTabColor = InSelectedTabColor;
	UnselectedTabColor = InUnselectedTabColor;
}

// Returns the current label text as a string.
FString UGameTypeTab::GetLabel() const
{
	if (!GameTypeLabel)
	{
		return FString();
	}
	return GameTypeLabel->GetText().ToString();
}

// Enables or disables the button to reflect whether campaigns exist for this game type.
void UGameTypeTab::SetInteractable(bool bInteractable)
{
	if (GameTypeTab)
	{
		GameTypeTab->SetIsEnabled(bInteractable);
	}
}

// Applies SelectedTabColor or UnselectedTabColor to the button background based on bSelected.
void UGameTypeTab::SetSelected(bool bSelected)
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
