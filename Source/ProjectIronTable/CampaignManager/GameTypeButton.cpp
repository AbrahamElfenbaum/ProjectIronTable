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

// Enables or disables the button to reflect whether campaigns exist for this game type.
void UGameTypeButton::SetInteractable(bool bInteractable)
{
	GameTypeTab->SetIsEnabled(bInteractable);
}

// Broadcasts OnGameTypeSelected with the current label text as the game type name.
void UGameTypeButton::OnGameTypeButtonClicked()
{
	OnGameTypeSelected.Broadcast(GameTypeLabel->GetText().ToString());
}
