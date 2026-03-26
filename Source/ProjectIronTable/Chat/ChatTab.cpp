// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatTab.h"
#include "ChatChannel.h"

void UChatTab::NativeConstruct()
{
	Super::NativeConstruct();

	if (TabButton)
	{
		TabButton->OnClicked.AddDynamic(this, &UChatTab::OnTabButtonClicked);
	}
}

void UChatTab::SetChannel(UChatChannel* InChannel)
{
	Channel = InChannel;
}

void UChatTab::SetLabel(const FString& Label)
{
	TabLabel->SetText(FText::FromString(Label));
}

void UChatTab::ShowNotification()
{
	NotificationIndicator->SetVisibility(ESlateVisibility::Visible);
}

void UChatTab::ClearNotification()
{
	NotificationIndicator->SetVisibility(ESlateVisibility::Collapsed);
}

void UChatTab::OnTabButtonClicked()
{
	OnTabClicked.Broadcast(Channel);
}
