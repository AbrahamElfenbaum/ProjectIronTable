// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatChannel.h"

#include "Components/ScrollBox.h"

#include "ChatEntry.h"
#include "MacroLibrary.h"

// Creates a UChatEntry widget with the given message, appends it to the scroll box, and scrolls to the bottom.
void UChatChannel::AddChatMessage(const FString& Message)
{
	if (!ChatEntryClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatChannel::AddChatMessage — ChatEntryClass is null; cannot create message entry"));
		return;
	}
	UChatEntry* ChatEntry = CreateWidget<UChatEntry>(this, ChatEntryClass);
	CHECK_IF_VALID(ChatEntry, );
	ChatEntry->Message = Message;
	if (ScrollBox)
	{
		ScrollBox->AddChild(ChatEntry);
		ScrollBox->ScrollToEnd();
	}
}

// Sets the entry class used when creating new message widgets.
void UChatChannel::SetChatEntryClass(TSubclassOf<UChatEntry> EntryClass)
{
	ChatEntryClass = EntryClass;
}

// Appends a previously saved message directly to the scroll box without triggering any routing or notification logic.
void UChatChannel::RestoreMessage(const FString& SenderName, const FString& Message)
{
	if (!ChatEntryClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatChannel::RestoreMessage — ChatEntryClass is null; cannot restore message entry"));
		return;
	}
	UChatEntry* ChatEntry = CreateWidget<UChatEntry>(this, ChatEntryClass);
	CHECK_IF_VALID(ChatEntry, );
	ChatEntry->Message = FString::Printf(TEXT("%s: %s"), *SenderName, *Message);
	if (ScrollBox)
	{
		ScrollBox->AddChild(ChatEntry);
		ScrollBox->ScrollToEnd();
	}
}
