// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatBox.h"

#include "Kismet/GameplayStatics.h"
#include "Components/EditableText.h"

#include "BaseChannel.h"
#include "BaseChannelTab.h"
#include "ChatChannel.h"
#include "ChatEntry.h"
#include "FunctionLibrary.h"
#include "MacroLibrary.h"
#include "SessionChatComponent.h"
#include "SessionInstance.h"
#include "SessionSave.h"

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

// Calls base to create and wire the channel, then sets ChatEntryClass and saves the tab name.
UBaseChannel* UChatBox::CreateChannel(const TArray<FString>& Participants)
{
	UBaseChannel* BaseChannel = Super::CreateChannel(Participants);
	CHECK_IF_VALID(BaseChannel, nullptr);

	UChatChannel* Channel = Cast<UChatChannel>(BaseChannel);
	if (!Channel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatBox::CreateChannel — Channel is not a UChatChannel"));
		return BaseChannel;
	}

	Channel->ChatEntryClass = ChatEntryClass;

	FString ParticipantsKey = UFunctionLibrary::MakeParticipantKey(Channel->Participants);
	USessionSave* SessionSave = UFunctionLibrary::LoadSessionSave(this);
	if (IsValid(SessionSave))
	{
		if (!SessionSave->ChatTabNames.Find(ParticipantsKey))
		{
			SessionSave->ChatTabNames.Add(ParticipantsKey, Channel->DisplayName);
		}
		UGameplayStatics::SaveGameToSlot(SessionSave, UFunctionLibrary::GetSessionSaveSlotName(GetGameInstance<USessionInstance>()), 0);
	}

	return Channel;
}

// Routes the message to the matching channel (creating one if needed), switching to it if the local player sent it.
void UChatBox::AddChatMessage(const FString& Message, const TArray<FString>& Participants, bool bIsSender)
{
	UBaseChannel* BaseChannel = FindOrCreateChannel(Participants);
	UChatChannel* CurrentChannel = Cast<UChatChannel>(BaseChannel);
	if (!CurrentChannel)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatBox::AddChatMessage — Channel is not a UChatChannel"));
		return;
	}

	CurrentChannel->AddChatMessage(Message);

	if (ClosedChannels.Contains(BaseChannel))
	{
		ReopenChannel(BaseChannel);
	}
	else if (bIsSender)
	{
		SwitchToChannel(BaseChannel);
	}
	else if (BaseChannel != ActiveChannel)
	{
		ChannelTabMap[BaseChannel]->ShowNotification();
	}
}

// Appends the given text to whatever is currently in the input field.
void UChatBox::AppendToInput(const FString& Text)
{
	FString Current = EditableText->GetText().ToString();
	EditableText->SetText(FText::FromString(Current + Text));
}

// Parses the input field for @mentions and sends a message to those recipients without switching the active channel.
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

// Stores the chat component reference for use when sending messages to the server.
void UChatBox::SetChatComponent(USessionChatComponent* InChatComponent)
{
	ChatComponentRef = InChatComponent;
}

// Binds the text committed delegate; base handles channel and button setup.
void UChatBox::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableText)
	{
		EditableText->OnTextCommitted.AddDynamic(this, &UChatBox::OnTextCommitted);
	}
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

// On Enter: parses @mentions and sends to the server. On focus loss: exits chat.
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

// Tokenizes Message on spaces; words prefixed with '@' are stripped into OutRecipients, the rest join into OutBody.
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

// Builds the display label from the participant list: "Server", "@Name", or "@Name +N".
FString UChatBox::CreateTabLabel(const TArray<FString>& Participants) const
{
	TArray<FString> Sorted = Participants;
	Sorted.Sort();

	if (Sorted.IsEmpty())
	{
		return TEXT("Server");
	}
	else if (Sorted.Num() == 1)
	{
		return TEXT("@") + Sorted[0];
	}
	return FString::Printf(TEXT("@%s +%d"), *Sorted[0], Sorted.Num() - 1);
}

// Save is handled directly in CreateChannel; no additional work needed here.
void UChatBox::SaveCreatedTab()
{
}

// Persists the renamed tab label to the session save using the participant key.
void UChatBox::OnChannelRenamed(UBaseChannelTab* Tab, const FString& NewName, const FString& ParticipantsKey)
{
	USessionSave* SessionSave = UFunctionLibrary::LoadSessionSave(this);
	if (IsValid(SessionSave))
	{
		SessionSave->ChatTabNames.Add(ParticipantsKey, NewName);
		UGameplayStatics::SaveGameToSlot(SessionSave, UFunctionLibrary::GetSessionSaveSlotName(GetGameInstance<USessionInstance>()), 0);
	}
}

// Clears the input field when the active channel is switched.
void UChatBox::OnChannelSwitched(UBaseChannel* Channel)
{
	EditableText->SetText(FText::GetEmpty());
}
