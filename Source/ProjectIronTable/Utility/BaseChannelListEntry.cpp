// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "BaseChannelListEntry.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "BaseChannel.h"
#include "MacroLibrary.h"

// Binds the entry button click delegate.
void UBaseChannelListEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (EntryButton)
	{
		EntryButton->OnClicked.AddDynamic(this, &UBaseChannelListEntry::OnEntryButtonClicked);
	}
}

// Broadcasts OnEntryClicked with the stored channel pointer.
void UBaseChannelListEntry::OnEntryButtonClicked()
{
	OnEntryClicked.Broadcast(Channel);
}

// Stores the channel reference and updates the label to the channel's display name.
void UBaseChannelListEntry::SetChannel(UBaseChannel* InChannel)
{
	CHECK_IF_VALID(InChannel, );
	Channel = InChannel;
	if (EntryLabel)
	{
		EntryLabel->SetText(FText::FromString(Channel->DisplayName));
	}
}
