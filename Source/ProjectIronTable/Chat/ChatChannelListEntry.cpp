// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatChannelListEntry.h"
#include "ChatChannel.h"

// Binds the entry button click delegate.
void UChatChannelListEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (EntryButton)
	{
		EntryButton->OnClicked.AddDynamic(this, &UChatChannelListEntry::OnEntryButtonClicked);
	}
}

// Broadcasts OnEntryClicked with the stored channel pointer.
void UChatChannelListEntry::OnEntryButtonClicked()
{
	OnEntryClicked.Broadcast(Channel);
}

// Stores the channel reference and updates the label to the channel's display name.
void UChatChannelListEntry::SetChannel(UChatChannel* InChannel)
{
	Channel = InChannel;
	EntryLabel->SetText(FText::FromString(Channel->DisplayName));
}