// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "BaseChannelPanel.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"

#include "BaseChannel.h"
#include "BaseChannelListEntry.h"
#include "BaseChannelTab.h"
#include "ContextMenu.h"
#include "FunctionLibrary.h"
#include "MacroLibrary.h"

// Binds the channel list button delegate, collapses the closed list, and creates the default server channel.
void UBaseChannelPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (ChannelListButton)
	{
		ChannelListButton->OnClicked.AddDynamic(this, &UBaseChannelPanel::OnChannelListButtonClicked);
	}

	if (ClosedChannelContainer)
	{
		ClosedChannelContainer->SetVisibility(ESlateVisibility::Collapsed);
	}

	SwitchToChannel(CreateChannel({}));
}

// Delegates scroll to the active channel.
void UBaseChannelPanel::Scroll(bool bUp)
{
	if (IsValid(ActiveChannel)) ActiveChannel->Scroll(bUp);
}

// Creates and wires a new channel and tab for the given participant list and returns the channel.
UBaseChannel* UBaseChannelPanel::CreateChannel(const TArray<FString>& Participants)
{
	FString Label = CreateTabLabel(Participants);
	TArray<FString> SortedParticipants = Participants;
	SortedParticipants.Sort();

	UBaseChannel* Channel = CreateWidget<UBaseChannel>(this, ChannelClass);
	CHECK_IF_VALID(Channel, nullptr);
	Channel->DisplayName = Label;
	Channel->Participants = SortedParticipants;

	Channels.Add(Channel);
	ChannelContainer->AddChild(Channel);

	UBaseChannelTab* Tab = CreateWidget<UBaseChannelTab>(this, TabClass);
	CHECK_IF_VALID(Tab, Channel);
	Tab->SetChannel(Channel);
	Tab->SetLabel(Label);
	Tab->OnTabClicked.AddDynamic(this, &UBaseChannelPanel::SwitchToChannel);
	Tab->OnTabRightClicked.AddDynamic(this, &UBaseChannelPanel::OnTabRightClickedHandler);
	Tab->OnTabRenamed.AddDynamic(this, &UBaseChannelPanel::OnTabRenamedHandler);

	ChannelTabMap.Add(Channel, Tab);
	TabBar->AddChild(Tab);

	SaveCreatedTab();

	return Channel;
}

// Returns the participant list of the active channel, or an empty array if no channel is active.
TArray<FString> UBaseChannelPanel::GetActiveChannelParticipants()
{
	if (IsValid(ActiveChannel))
	{
		return ActiveChannel->Participants;
	}
	else
	{
		return {};
	}
}

// Searches existing channels for a participant-list match; creates and returns a new channel if none found.
UBaseChannel* UBaseChannelPanel::FindOrCreateChannel(const TArray<FString>& Participants)
{
	FString IncomingKey = UFunctionLibrary::MakeParticipantKey(Participants);

	for (UBaseChannel* Channel : Channels)
	{
		if (UFunctionLibrary::MakeParticipantKey(Channel->Participants) == IncomingKey)
		{
			return Channel;
		}
	}
	return CreateChannel(Participants);
}

// Looks up and returns the tab for the given channel, or nullptr if not found.
UBaseChannelTab* UBaseChannelPanel::GetTabForChannel(UBaseChannel* Channel) const
{
	UBaseChannelTab* const* Tab = ChannelTabMap.Find(Channel);
	return Tab ? *Tab : nullptr;
}

// Sets the old active tab interactable, switches to the new channel, and calls OnChannelSwitched.
void UBaseChannelPanel::SwitchToChannel(UBaseChannel* Channel)
{
	if (IsValid(ActiveChannel))
	{
		ChannelTabMap[ActiveChannel]->SetInteractable(true);
	}

	ActiveChannel = Channel;
	ChannelTabMap[ActiveChannel]->SetInteractable(false);
	ChannelContainer->SetActiveWidget(Channel);
	ChannelTabMap[Channel]->ClearNotification();
	OnChannelSwitched(Channel);
}

// Toggles the closed channel list panel between visible and collapsed.
void UBaseChannelPanel::OnChannelListButtonClicked()
{
	if (ClosedChannelContainer->GetVisibility() == ESlateVisibility::Collapsed)
	{
		ClosedChannelContainer->SetVisibility(ESlateVisibility::Visible);
	}
	else if (ClosedChannelContainer->GetVisibility() == ESlateVisibility::Visible)
	{
		ClosedChannelContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// Adds the channel to the closed set, collapses its tab, and falls back to the server channel if it was active.
void UBaseChannelPanel::CloseChannel(UBaseChannel* Channel)
{
	ClosedChannels.Add(Channel);

	ChannelTabMap[Channel]->SetVisibility(ESlateVisibility::Collapsed);

	if (ActiveChannel == Channel)
	{
		SwitchToChannel(Channels[0]);
	}

	RefreshChannelList();
}

// Removes the channel from the closed set, restores its tab, switches to it, and collapses the list panel.
void UBaseChannelPanel::ReopenChannel(UBaseChannel* Channel)
{
	ClosedChannels.Remove(Channel);
	ChannelTabMap[Channel]->SetVisibility(ESlateVisibility::Visible);
	SwitchToChannel(Channel);
	RefreshChannelList();
	ClosedChannelContainer->SetVisibility(ESlateVisibility::Collapsed);
}

// Spawns a context menu at the cursor with Rename and Close options when a non-Server tab is right-clicked.
void UBaseChannelPanel::OnTabRightClickedHandler(UBaseChannel* Channel)
{
	if (Channel->Participants.IsEmpty())
	{
		return;
	}

	if (IsValid(ActiveContextMenuRef))
	{
		ActiveContextMenuRef->CloseMenu();
	}

	UContextMenu* ContextMenu = CreateWidget<UContextMenu>(this, ContextMenuClass);
	CHECK_IF_VALID(ContextMenu, );

	FContextMenuOption RenameOption;
	RenameOption.ButtonName = TEXT("Rename");
	RenameOption.OnClicked.BindLambda([this, Channel]()
		{
			GetTabForChannel(Channel)->EnterRenameMode();
		});

	FContextMenuOption CloseOption;
	CloseOption.ButtonName = TEXT("Close");
	CloseOption.OnClicked.BindLambda([this, Channel]()
		{
			CloseChannel(Channel);
		});

	ContextMenu->SetMenuOptions({ RenameOption, CloseOption });
	ContextMenu->AddToViewport();

	GET_OWNING_PC(PC, );
	FVector2D MousePosition;
	PC->GetMousePosition(MousePosition.X, MousePosition.Y);
	ContextMenu->SetMenuPosition(MousePosition);
	ActiveContextMenuRef = ContextMenu;
}

// Resolves the channel from the tab, computes the participant key, and delegates to OnChannelRenamed.
void UBaseChannelPanel::OnTabRenamedHandler(UBaseChannelTab* Tab, const FString& NewName)
{
	UBaseChannel* Channel = Cast<UBaseChannel>(Tab->GetChannel());
	if (!Channel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBaseChannelPanel::OnTabRenamedHandler — Channel is not a UBaseChannel"));
		return;
	}
	FString ParticipantsKey = UFunctionLibrary::MakeParticipantKey(Channel->Participants);
	OnChannelRenamed(Tab, NewName, ParticipantsKey);
}

// Returns an empty string; subclasses override to provide their label-building logic.
FString UBaseChannelPanel::CreateTabLabel(const TArray<FString>& Participants) const
{
	return FString();
}

// No-op in base; subclasses override to persist tab data after channel creation.
void UBaseChannelPanel::SaveCreatedTab()
{
}

// No-op in base; subclasses override to persist the new tab name after a rename.
void UBaseChannelPanel::OnChannelRenamed(UBaseChannelTab* Tab, const FString& NewName, const FString& ParticipantsKey)
{
}

// No-op in base; subclasses override for panel-specific behavior after a channel switch.
void UBaseChannelPanel::OnChannelSwitched(UBaseChannel* Channel)
{
}

// Clears and repopulates the closed channel list panel from the current ClosedChannels set.
void UBaseChannelPanel::RefreshChannelList()
{
	ClosedChannelContainer->ClearChildren();
	for (UBaseChannel* Channel : ClosedChannels)
	{
		UBaseChannelListEntry* Entry = CreateWidget<UBaseChannelListEntry>(this, ChannelListEntryClass);
		if (!IsValid(Entry))
		{
			UE_LOG(LogTemp, Warning, TEXT("UBaseChannelPanel::RefreshChannelList — Failed to create ChannelListEntry widget"));
			continue;
		}
		Entry->SetChannel(Channel);
		Entry->OnEntryClicked.AddDynamic(this, &UBaseChannelPanel::ReopenChannel);
		ClosedChannelContainer->AddChild(Entry);
	}
}
