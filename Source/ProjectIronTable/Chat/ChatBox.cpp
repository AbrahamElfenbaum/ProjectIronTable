// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatBox.h"
#include "ChatEntry.h"
#include "ChatTab.h"
#include "ChatChannel.h"
#include "GameplayHUDComponent.h"
#include "GameFramework/PlayerState.h"

// Caches the HUD component reference, binds the text committed delegate, and creates the default server channel.
void UChatBox::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (PC)
	{
		HUDComponentRef = Cast<UGameplayHUDComponent>(
			PC->GetComponentByClass(UGameplayHUDComponent::StaticClass()));
	}

	EditableText->OnTextCommitted.AddDynamic(this, &UChatBox::OnTextCommitted);
	SwitchToChannel(CreateChannel({}));
}

// Focuses the chat box when the widget is clicked while not already focused.
FReply UChatBox::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bChatFocused)
	{
		FocusChat();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

// Delegates scroll to the active channel.
void UChatBox::Scroll(bool bUp)
{
	if (ActiveChannel) ActiveChannel->Scroll(bUp);
}

// Sets keyboard focus on the editable text field and switches to UI-only input mode.
void UChatBox::FocusChat()
{
	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (PC)
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
	EditableText->SetText(FText::GetEmpty());
	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (PC)
	{
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

// Creates and registers a new channel and its corresponding tab, returning the new channel.
UChatChannel* UChatBox::CreateChannel(TArray<FString> Participants)
{
	//Build the label based on the participants
	FString Label;

	if (Participants.IsEmpty())
	{
		Label = TEXT("Server");
	}
	else if (Participants.Num() == 1)
	{
		Label = TEXT("@") + Participants[0];
	}
	else
	{
		Label = FString::Printf(TEXT("@%s +%d"), *Participants[0], Participants.Num() - 1);
	}

	//Channel widget creation and setup
	UChatChannel* Channel = CreateWidget<UChatChannel>(GetOwningPlayer(), ChannelClass);
	Channel->ChatEntryClass = ChatEntryClass;
	Channel->DisplayName = Label;
	Channel->Participants = Participants;

	Channels.Add(Channel);
	ChannelContainer->AddChild(Channel);

	//Tab widget creation and setup
	UChatTab* Tab = CreateWidget<UChatTab>(GetOwningPlayer(), TabClass);
	Tab->SetChannel(Channel);
	Tab->SetLabel(Label);
	Tab->OnTabClicked.AddDynamic(this, &UChatBox::SwitchToChannel);

	ChannelTabMap.Add(Channel, Tab);

	TabBar->AddChild(Tab);

	return Channel;
}

// Makes the given channel visible in the switcher and clears its unread notification.
void UChatBox::SwitchToChannel(UChatChannel* Channel)
{
	ActiveChannel = Channel;
	ChannelContainer->SetActiveWidget(Channel);
	ChannelTabMap[Channel]->ClearNotification();
}

// Routes the message to the matching channel (creating one if needed), switching to it if the local player sent it.
void UChatBox::AddChatMessage(const FString& Message, TArray<FString> Participants, bool bIsSender)
{
	UChatChannel* CurrentChannel = nullptr;

	for (UChatChannel* Channel : Channels)
	{
		bool bChannelFound = true;
		if (Channel->Participants.Num() == Participants.Num())
		{
			for (FString Participant : Participants)
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

	CurrentChannel->AddChatMessage(Message);

	if (bIsSender)
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
	if (ActiveChannel)
	{
		return ActiveChannel->Participants;
	}
	else
	{
		return {};
	}
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
			if (PC && PC->PlayerState)
			{
				TArray<FString> Words;
				Message.ParseIntoArray(Words, TEXT(" "), 1);

				TArray<FString> Recipients;
				TArray<FString> MessageArray;

				for(FString Word : Words)
				{
					if (Word.StartsWith(TEXT("@")))
					{
						Word.RemoveAt(0);
						Recipients.Add(Word);
					}
					else
					{
						MessageArray.Add(Word);
					}
				}

				Message = FString::Join(MessageArray, TEXT(" "));

				FString PlayerName = PC->PlayerState->GetPlayerName();
				FString FullMessage = FString::Printf(TEXT("%s: %s"), *PlayerName, *Message);
				HUDComponentRef->SendChatMessageOnServer(FullMessage, Recipients);
			}

			EditableText->SetText(FText::GetEmpty());

			FocusChat();
		}
	}
	else if (CommitMethod == ETextCommit::OnUserMovedFocus ||
			 CommitMethod == ETextCommit::OnCleared)
	{
		ExitChat();
	}
}
