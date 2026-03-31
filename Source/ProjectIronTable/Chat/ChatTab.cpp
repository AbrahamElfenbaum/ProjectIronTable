// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatTab.h"
#include "ChatChannel.h"

// Binds the tab button click delegate.
void UChatTab::NativeConstruct()
{
	Super::NativeConstruct();

	if (TabButton)
	{
		TabButton->OnClicked.AddDynamic(this, &UChatTab::OnTabButtonClicked);
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

void UChatTab::SetInteractable(bool bInteractable)
{
	TabButton->SetIsEnabled(bInteractable);
}
