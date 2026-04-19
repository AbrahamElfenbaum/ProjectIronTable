// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatBox.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/EditableText.h"

#include "BaseChannel.h"
#include "BaseChannelTab.h"
#include "ChatTab.h"
#include "ChatEntry.h"
#include "ChatChannel.h"
#include "ChatChannelListEntry.h"
#include "ContextMenu.h"
#include "FunctionLibrary.h"
#include "MacroLibrary.h"
#include "SessionChatComponent.h"
#include "SessionInstance.h"
#include "SessionSave.h"

// Caches the HUD component reference, binds the text committed delegate, and creates the default server channel.
void UChatBox::NativeConstruct()
{
	Super::NativeConstruct();

	GET_OWNING_PC(PC, );

	if (ChannelListButton)
	{
		ChannelListButton->OnClicked.AddDynamic(this, &UChatBox::OnChannelListButtonClicked);
	}

	if (EditableText)
	{
		EditableText->OnTextCommitted.AddDynamic(this, &UChatBox::OnTextCommitted);
	}

	if (ClosedChannelContainer)
	{
		ClosedChannelContainer->SetVisibility(ESlateVisibility::Collapsed);
	}

	SwitchToChannel(CreateChannel({}));
}

// Focuses the chat box when the widget is clicked while not already focused.
FReply UChatBox::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bChatFocused)
	{
		FocusChat();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

// Delegates scroll to the active channel.
void UChatBox::Scroll(bool bUp)
{
	if (IsValid(ActiveChannel)) ActiveChannel->Scroll(bUp);
}

// Sets keyboard focus on the editable text field and switches to UI-only input mode.
void UChatBox::FocusChat()
{
	GET_OWNING_PC(PC, );
	EditableText->SetUserFocus(PC);
	bChatFocused = true;
	EditableText->SetIsEnabled(true);
	PC->SetInputMode(FInputModeUIOnly());
}

// Clears and disables the input field and restores game-and-UI input mode.
void UChatBox::ExitChat()
{
	bChatFocused = false;
	EditableText->SetIsEnabled(false);
	GET_OWNING_PC(PC, );
	PC->SetInputMode(FInputModeGameAndUI());
	PC->bShowMouseCursor = true;
}

// Creates and registers a new channel and its corresponding tab, returning the new channel.
UChatChannel* UChatBox::CreateChannel(const TArray<FString>& Participants)
{
	//Build the label based on the participants
	FString Label;

	TArray<FString> SortedParticipants = Participants;
	SortedParticipants.Sort();

	if (SortedParticipants.IsEmpty())
	{
		Label = TEXT("Server");
	}
	else if (SortedParticipants.Num() == 1)
	{
		Label = TEXT("@") + SortedParticipants[0];
	}
	else
	{
		Label = FString::Printf(TEXT("@%s +%d"), *SortedParticipants[0], SortedParticipants.Num() - 1);
	}

	//Channel widget creation and setup
	UChatChannel* Channel = CreateWidget<UChatChannel>(this, ChannelClass);
	CHECK_IF_VALID(Channel, nullptr);
	Channel->ChatEntryClass = ChatEntryClass;
	Channel->DisplayName = Label;
	Channel->Participants = SortedParticipants;

	Channels.Add(Channel);
	ChannelContainer->AddChild(Channel);

	//Tab widget creation and setup
	UChatTab* Tab = CreateWidget<UChatTab>(this, TabClass);
	CHECK_IF_VALID(Tab, Channel);
	Tab->SetChannel(Channel);
	Tab->SetLabel(Label);
	Tab->OnTabClicked.AddDynamic(this, &UChatBox::SwitchToChannel);
	Tab->OnTabRightClicked.AddDynamic(this, &UChatBox::OnTabRightClickedHandler);
	Tab->OnTabRenamed.AddDynamic(this, &UChatBox::OnTabRenamedHandler);


	ChannelTabMap.Add(Channel, Tab);

	TabBar->AddChild(Tab);

	FString sSortedParticipants = UFunctionLibrary::MakeParticipantKey(SortedParticipants);

	USessionSave* SessionSave = UFunctionLibrary::LoadSessionSave(this);
	if (IsValid(SessionSave))
	{
		auto ChannelName = SessionSave->ChatTabNames.Find(sSortedParticipants);
		if (!ChannelName)
		{
			SessionSave->ChatTabNames.Add(sSortedParticipants, Label);
		}
		UGameplayStatics::SaveGameToSlot(SessionSave, UFunctionLibrary::GetSessionSaveSlotName(GetGameInstance<USessionInstance>()), 0);
	}

	return Channel;
}

// Makes the given channel visible in the switcher and clears its unread notification.
void UChatBox::SwitchToChannel(UBaseChannel* Channel)
{
	UChatChannel* ChatChannel = Cast<UChatChannel>(Channel);
	if (!ChatChannel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatBox::SwitchToChannel — Channel is not a UChatChannel"));
		return;
	}

	if (IsValid(ActiveChannel))
	{
		ChannelTabMap[ActiveChannel]->SetInteractable(true);
	}

	ActiveChannel = ChatChannel;
	ChannelTabMap[ActiveChannel]->SetInteractable(false);
	ChannelContainer->SetActiveWidget(ChatChannel);
	ChannelTabMap[ChatChannel]->ClearNotification();
	EditableText->SetText(FText::GetEmpty());
}

// Routes the message to the matching channel (creating one if needed), switching to it if the local player sent it.
void UChatBox::AddChatMessage(const FString& Message, const TArray<FString>& Participants, bool bIsSender)
{
	UChatChannel* CurrentChannel = FindOrCreateChannel(Participants);

	CurrentChannel->AddChatMessage(Message);

	if (ClosedChannels.Contains(CurrentChannel))
	{
		ReopenChannel(CurrentChannel);
	}
	else if (bIsSender)
	{
		SwitchToChannel(CurrentChannel);
	}
	else if (CurrentChannel != ActiveChannel)
	{
		ChannelTabMap[CurrentChannel]->ShowNotification();
	}
}

// Appends the given text to whatever is currently in the input field.
void UChatBox::AppendToInput(const FString& Text)
{
	FString Current = EditableText->GetText().ToString();
	EditableText->SetText(FText::FromString(Current + Text));
}

// Returns the participant list of the active channel, or an empty array if no channel is active.
TArray<FString> UChatBox::GetActiveChannelParticipants()
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

// Parses the input field for @mentions and sends a message to those recipients without switching the active channel, 
// used for rolling in private channels without losing context.
void UChatBox::TrySendPrivateRollMessage()
{
	FString Body;
	TArray<FString> Recipients;
	ParseMentions(EditableText->GetText().ToString(), Recipients, Body);

	if (Body.IsEmpty()) Body = TEXT("Rolling...");

	FString PlayerName = UFunctionLibrary::GetLocalPlayerName(this);

	if (!Recipients.IsEmpty())
	{
		Recipients.Remove(PlayerName);

		CHECK_IF_VALID(ChatComponentRef, );

		FString FullMessage = FString::Printf(TEXT("%s: %s"), *PlayerName, *Body);
		ChatComponentRef->SendChatMessageOnServer(FullMessage, Recipients);
		EditableText->SetText(FText::GetEmpty());
	}
}

// Searches existing channels for a participant-list match; creates and returns a new channel if none is found.
UChatChannel* UChatBox::FindOrCreateChannel(const TArray<FString>& Participants)
{
	FString IncomingKey = UFunctionLibrary::MakeParticipantKey(Participants);

	for (UChatChannel* Channel : Channels)
	{
		if (UFunctionLibrary::MakeParticipantKey(Channel->Participants) == IncomingKey)
		{
			return Channel;
		}
	}
	return CreateChannel(Participants);	
}

// Looks up and returns the tab for the given channel, or nullptr if not found.
UChatTab* UChatBox::GetTabForChannel(UChatChannel* Channel) const
{
	UChatTab* const* Tab = ChannelTabMap.Find(Channel);
	return Tab ? *Tab : nullptr;
}

// Stores the chat component reference for use when sending messages to the server.
void UChatBox::SetChatComponent(USessionChatComponent* InChatComponent)
{
	ChatComponentRef = InChatComponent;
}

// On Enter: parses @mentions from the message and sends it to the server. On focus loss: exits chat.
void UChatBox::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		FString Message = Text.ToString();

		if (!Message.IsEmpty())
		{
			FString Body;
			TArray<FString> Recipients;
			ParseMentions(EditableText->GetText().ToString(), Recipients, Body);

			FString PlayerName = UFunctionLibrary::GetLocalPlayerName(this);

			// If no @recipients were typed but the active channel is private,
			// automatically route to that channel's participants so replies
			// don't fall back to the public server channel.
			if (Recipients.IsEmpty() && ActiveChannel && !ActiveChannel->Participants.IsEmpty())
			{
				Recipients = ActiveChannel->Participants;
				Recipients.Remove(PlayerName);
			}

			CHECK_IF_VALID(ChatComponentRef, );

			FString FullMessage = FString::Printf(TEXT("%s: %s"), *PlayerName, *Body);
			ChatComponentRef->SendChatMessageOnServer(FullMessage, Recipients);

			EditableText->SetText(FText::GetEmpty());

			bPendingRefocus = true;
			FocusChat();
		}
	}
	else if (CommitMethod == ETextCommit::OnUserMovedFocus ||
		CommitMethod == ETextCommit::OnCleared)
	{
		if (bPendingRefocus)
		{
			bPendingRefocus = false;
			FocusChat();
		}
		else
		{
			ExitChat();
		}
	}
}

// Toggles the closed channel list panel between visible and collapsed.
void UChatBox::OnChannelListButtonClicked()
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

// Hides the tab for the given channel, adds it to the closed set, and falls back to the Server channel if it was active.
void UChatBox::CloseChannel(UChatChannel* Channel)
{
	ClosedChannels.Add(Channel);

	ChannelTabMap[Channel]->SetVisibility(ESlateVisibility::Collapsed);

	if (ActiveChannel == Channel)
	{
		SwitchToChannel(Channels[0]);
	}

	RefreshChannelList();
}

// Removes the given channel from the closed set, restores its tab, switches to it, and collapses the list panel.
void UChatBox::ReopenChannel(UBaseChannel* Channel)
{
	UChatChannel* ChatChannel = Cast<UChatChannel>(Channel);
	if (!ChatChannel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatBox::ReopenChannel — Channel is not a UChatChannel"));
		return;
	}

	ClosedChannels.Remove(ChatChannel);
	ChannelTabMap[ChatChannel]->SetVisibility(ESlateVisibility::Visible);
	SwitchToChannel(ChatChannel);
	RefreshChannelList();
	ClosedChannelContainer->SetVisibility(ESlateVisibility::Collapsed);
}

// Clears and repopulates the closed channel list panel from the current ClosedChannels set.
void UChatBox::RefreshChannelList()
{
	ClosedChannelContainer->ClearChildren();
	for (UChatChannel* Channel : ClosedChannels)
	{
		UChatChannelListEntry* Entry = CreateWidget<UChatChannelListEntry>(this, ChannelListEntryClass);
		if (!IsValid(Entry))
		{
			UE_LOG(LogTemp, Warning, TEXT("UChatBox::RefreshChannelList — Failed to create ChannelListEntry widget"));
			continue;
		}
		Entry->SetChannel(Channel);
		Entry->OnEntryClicked.AddDynamic(this, &UChatBox::ReopenChannel);
		ClosedChannelContainer->AddChild(Entry);
	}
}

// Spawns a context menu at the cursor with Rename and Close options when a non-Server tab is right-clicked.
void UChatBox::OnTabRightClickedHandler(UBaseChannel* Channel)
{
	UChatChannel* ChatChannel = Cast<UChatChannel>(Channel);
	if (!ChatChannel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatBox::OnTabRightClickedHandler — Channel is not a UChatChannel"));
		return;
	}

	if (ChatChannel->Participants.IsEmpty())
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
	RenameOption.OnClicked.BindLambda([this, ChatChannel]()
		{
			GetTabForChannel(ChatChannel)->EnterRenameMode();
		});

	FContextMenuOption CloseOption;

	CloseOption.ButtonName = TEXT("Close");
	CloseOption.OnClicked.BindLambda([this, ChatChannel]()
		{
			CloseChannel(ChatChannel);
		});

	ContextMenu->SetMenuOptions({ RenameOption, CloseOption });
	ContextMenu->AddToViewport();

	GET_OWNING_PC(PC, );
	FVector2D MousePosition;
	PC->GetMousePosition(MousePosition.X, MousePosition.Y);
	ContextMenu->SetMenuPosition(MousePosition);
	ActiveContextMenuRef = ContextMenu;
}

// Updates the session save with the new name for the given tab's channel, using the participant list as the key to persist across sessions.
void UChatBox::OnTabRenamedHandler(UBaseChannelTab* Tab, const FString& NewName)
{
	USessionSave* SessionSave = UFunctionLibrary::LoadSessionSave(this);
	if (IsValid(SessionSave))
	{
		UChatTab* ChatTab = Cast<UChatTab>(Tab);
		if (!ChatTab)
		{
			UE_LOG(LogTemp, Warning, TEXT("UChatBox::OnTabRenamedHandler — Tab is not a UChatTab"));
			return;
		}

		UChatChannel* ChatChannel = Cast<UChatChannel>(Tab->GetChannel());
		if (!ChatChannel)
		{
			UE_LOG(LogTemp, Warning, TEXT("UChatBox::OnTabRenamedHandler — Channel is not a UChatChannel"));
			return;
		}
		FString ParticipantsKey = UFunctionLibrary::MakeParticipantKey(ChatChannel->Participants);
		SessionSave->ChatTabNames.Add(ParticipantsKey, NewName);
		UGameplayStatics::SaveGameToSlot(SessionSave, UFunctionLibrary::GetSessionSaveSlotName(GetGameInstance<USessionInstance>()), 0);
	}
}

// Tokenizes Message on spaces; words prefixed with '@' are stripped and added to OutRecipients, the rest join into OutBody.
void UChatBox::ParseMentions(const FString& Message, TArray<FString>& OutRecipients, FString& OutBody) const
{
	TArray<FString> Words;
	Message.ParseIntoArray(Words, TEXT(" "), 1);

	TArray<FString> MessageArray;

	for (const FString& Word : Words)
	{
		if (Word.StartsWith(TEXT("@")))
		{
			OutRecipients.Add(Word.RightChop(1));
		}
		else
		{
			MessageArray.Add(Word);
		}
	}
	OutBody = FString::Join(MessageArray, TEXT(" "));
}
