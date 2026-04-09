// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatTab.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "ChatChannel.h"

// Binds the tab button click delegate.
void UChatTab::NativeConstruct()
{
	Super::NativeConstruct();

	if (TabButton)
	{
		TabButton->OnClicked.AddDynamic(this, &UChatTab::OnTabButtonClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UChatTab::OnCloseButtonClicked);
	}
}

// Stores the channel reference used when broadcasting tab click events.
void UChatTab::SetChannel(UChatChannel* InChannel)
{
	Channel = InChannel;
}

// Updates the tab label text to the given string.
void UChatTab::SetLabel(const FString& Label)
{
	TabLabel->SetText(FText::FromString(Label));
}

// Makes the notification indicator visible.
void UChatTab::ShowNotification()
{
	NotificationIndicator->SetVisibility(ESlateVisibility::Visible);
}

// Hides the notification indicator.
void UChatTab::ClearNotification()
{
	NotificationIndicator->SetVisibility(ESlateVisibility::Collapsed);
}

// Broadcasts OnTabClicked with the stored channel pointer.
void UChatTab::OnTabButtonClicked()
{
	OnTabClicked.Broadcast(Channel);
}

// Broadcasts OnTabClosed with the stored channel pointer.
void UChatTab::OnCloseButtonClicked()
{
	OnTabClosed.Broadcast(Channel);
}

// Enables or disables the tab button.
void UChatTab::SetInteractable(bool bInteractable)
{
	TabButton->SetIsEnabled(bInteractable);
}

// Shows or hides the close button.
void UChatTab::SetCloseable(bool bShowButton)
{
	if (bShowButton)
	{
		CloseButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		CloseButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}
