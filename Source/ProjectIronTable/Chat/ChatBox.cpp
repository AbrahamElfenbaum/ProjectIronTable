// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatBox.h"

#include "Kismet/GameplayStatics.h"

#include "GameFramework/PlayerState.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/EditableText.h"

#include "ChatEntry.h"
#include "ChatTab.h"
#include "ChatChannel.h"
#include "ChatChannelListEntry.h"
#include "SessionHUDComponent.h"
#include "SessionInstance.h"
#include "SessionSave.h"

// Caches the HUD component reference, binds the text committed delegate, and creates the default server channel.
void UChatBox::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (IsValid(PC))
	{
		HUDComponentRef = Cast<USessionHUDComponent>(
			PC->GetComponentByClass(USessionHUDComponent::StaticClass()));
		if (!IsValid(HUDComponentRef))
		{
			UE_LOG(LogTemp, Warning, TEXT("UChatBox::NativeConstruct — Failed to find SessionHUDComponent on PlayerController"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatBox::NativeConstruct — GetOwningPlayer did not return a PlayerController"));
	}

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
	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (IsValid(PC))
	{
		EditableText->SetUserFocus(PC);
		bChatFocused = true;
		EditableText->SetIsEnabled(true);
		PC->SetInputMode(FInputModeUIOnly());
	}
}

// Clears and disables the input field and restores game-and-UI input mode.
void UChatBox::ExitChat()
{
	bChatFocused = false;
	EditableText->SetIsEnabled(false);
	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (IsValid(PC))
	{
		PC->SetInputMode(FInputModeGameAndUI());
		PC->bShowMouseCursor = true;
	}
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
	if (!IsValid(Channel))
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatBox::CreateChannel — Failed to create ChatChannel widget"));
		return nullptr;
	}
	Channel->ChatEntryClass = ChatEntryClass;
	Channel->DisplayName = Label;
	Channel->Participants = SortedParticipants;

	Channels.Add(Channel);
	ChannelContainer->AddChild(Channel);

	//Tab widget creation and setup
	UChatTab* Tab = CreateWidget<UChatTab>(this, TabClass);
	if (!IsValid(Tab))
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatBox::CreateChannel — Failed to create ChatTab widget"));
		return Channel;
	}
	Tab->SetChannel(Channel);
	Tab->SetLabel(Label);
	Tab->OnTabClicked.AddDynamic(this, &UChatBox::SwitchToChannel);
	Tab->OnTabClosed.AddDynamic(this, &UChatBox::CloseChannel);
	Tab->SetCloseable(!SortedParticipants.IsEmpty());

	ChannelTabMap.Add(Channel, Tab);

	TabBar->AddChild(Tab);

	FString sSortedParticipants = FString::Join(SortedParticipants, TEXT("|"));

	USessionInstance* SessionInstance = GetGameInstance<USessionInstance>();
	if (IsValid(SessionInstance))
	{
		USessionSave* SessionSave = Cast<USessionSave>(UGameplayStatics::LoadGameFromSlot(FString::Printf(TEXT("Session_%s"),
			*SessionInstance->GetSessionID().ToString()), 0));

		if (IsValid(SessionSave))
		{
			auto ChannelName = SessionSave->ChatTabNames.Find(sSortedParticipants);
			if (!ChannelName)
			{
				ChannelName = &SessionSave->ChatTabNames.Add(sSortedParticipants, Label);
			}
			UGameplayStatics::SaveGameToSlot(SessionSave, FString::Printf(TEXT("Session_%s"), *SessionInstance->GetSessionID().ToString()), 0);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UChatBox::CreateChannel — Failed to load session save; message will not be persisted"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatBox::CreateChannel — SessionInstance is null; message will not be persisted"));
	}

	return Channel;
}

// Makes the given channel visible in the switcher and clears its unread notification.
void UChatBox::SwitchToChannel(UChatChannel* Channel)
{
	if (IsValid(ActiveChannel))
	{
		ChannelTabMap[ActiveChannel]->SetInteractable(true);
	}
	ActiveChannel = Channel;
	ChannelTabMap[ActiveChannel]->SetInteractable(false);
	ChannelContainer->SetActiveWidget(Channel);
	ChannelTabMap[Channel]->ClearNotification();
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
	FString Message = EditableText->GetText().ToString();

	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (IsValid(PC) && IsValid(PC->PlayerState))
	{
		TArray<FString> Words;
		Message.ParseIntoArray(Words, TEXT(" "), 1);

		TArray<FString> Recipients;
		TArray<FString> MessageArray;

		for (const FString& Word : Words)
		{
			if (Word.StartsWith(TEXT("@")))
			{
				FString Name = Word.RightChop(1);
				Recipients.Add(Name);
			}
			else
			{
				MessageArray.Add(Word);
			}
		}

		if (MessageArray.IsEmpty())
		{
			Message = TEXT("Rolling...");
		}
		else
		{
			Message = FString::Join(MessageArray, TEXT(" "));
		}

		FString PlayerName = PC->PlayerState->GetPlayerName();

		if (!Recipients.IsEmpty())
		{
			Recipients.Remove(PlayerName);

			if (!IsValid(HUDComponentRef))
			{
				UE_LOG(LogTemp, Warning, TEXT("UChatBox::TrySendPrivateRollMessage — HUDComponentRef is null"));
				return;
			}

			FString FullMessage = FString::Printf(TEXT("%s: %s"), *PlayerName, *Message);
			HUDComponentRef->SendChatMessageOnServer(FullMessage, Recipients);
			EditableText->SetText(FText::GetEmpty());
		}
	}
}

// Searches existing channels for a participant-list match; creates and returns a new channel if none is found.
UChatChannel* UChatBox::FindOrCreateChannel(const TArray<FString>& Participants)
{
	UChatChannel* CurrentChannel = nullptr;

	for (UChatChannel* Channel : Channels)
	{
		bool bChannelFound = true;
		if (Channel->Participants.Num() == Participants.Num())
		{
			for (const FString& Participant : Participants)
			{
				if (!Channel->Participants.Contains(Participant))
				{
					bChannelFound = false;
					break;
				}
			}
			if (bChannelFound)
			{
				CurrentChannel = Channel;
				break;
			}
		}
	}

	if (!CurrentChannel)
	{
		CurrentChannel = CreateChannel(Participants);
	}

	return CurrentChannel;
}

// Looks up and returns the tab for the given channel, or nullptr if not found.
UChatTab* UChatBox::GetTabForChannel(UChatChannel* Channel) const
{
	UChatTab* const* Tab = ChannelTabMap.Find(Channel);
	return Tab ? *Tab : nullptr;
}

// On Enter: parses @mentions from the message and sends it to the server. On focus loss: exits chat.
void UChatBox::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		FString Message = Text.ToString();

		if (!Message.IsEmpty())
		{
			APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
			if (IsValid(PC) && IsValid(PC->PlayerState))
			{
				TArray<FString> Words;
				Message.ParseIntoArray(Words, TEXT(" "), 1);

				TArray<FString> Recipients;
				TArray<FString> MessageArray;

				for (const FString& Word : Words)
				{
					if (Word.StartsWith(TEXT("@")))
					{
						Recipients.Add(Word.RightChop(1));
					}
					else
					{
						MessageArray.Add(Word);
					}
				}

				Message = FString::Join(MessageArray, TEXT(" "));

				FString PlayerName = PC->PlayerState->GetPlayerName();

				// If no @recipients were typed but the active channel is private,
				// automatically route to that channel's participants so replies
				// don't fall back to the public server channel.
				if (Recipients.IsEmpty() && ActiveChannel && !ActiveChannel->Participants.IsEmpty())
				{
					Recipients = ActiveChannel->Participants;
					Recipients.Remove(PlayerName);
				}

				if (!IsValid(HUDComponentRef))
				{
					UE_LOG(LogTemp, Warning, TEXT("UChatBox::OnTextCommitted — HUDComponentRef is null"));
					return;
				}

				FString FullMessage = FString::Printf(TEXT("%s: %s"), *PlayerName, *Message);
				HUDComponentRef->SendChatMessageOnServer(FullMessage, Recipients);
			}

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
void UChatBox::ReopenChannel(UChatChannel* Channel)
{
	ClosedChannels.Remove(Channel);

	ChannelTabMap[Channel]->SetVisibility(ESlateVisibility::Visible);

	SwitchToChannel(Channel);

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
