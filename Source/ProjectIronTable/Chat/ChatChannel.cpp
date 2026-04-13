// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatChannel.h"

#include "Components/ScrollBox.h"

#include "ChatEntry.h"

// Creates a UChatEntry widget with the given message, appends it to the scroll box, and scrolls to the bottom.
void UChatChannel::AddChatMessage(const FString& Message)
{
	if (!ChatEntryClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatChannel::AddChatMessage — ChatEntryClass is null; cannot create message entry"));
		return;
	}
	UChatEntry* ChatEntry = CreateWidget<UChatEntry>(this, ChatEntryClass);
	if (!IsValid(ChatEntry))
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatChannel::AddChatMessage — Failed to create ChatEntry widget"));
		return;
	}
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

// Adjusts the scroll offset by ScrollMultiplier in the requested direction, clamped to valid range.
void UChatChannel::Scroll(bool bUp)
{
	if (!ScrollBox) return;

	int32 ScrollDirection = bUp ? 1 : -1;
	ScrollBox->SetScrollOffset(
		FMath::Clamp(
			ScrollBox->GetScrollOffset() + (ScrollMultiplier * ScrollDirection),
			0.0f,
			ScrollBox->GetScrollOffsetOfEnd()));
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
	if (!IsValid(ChatEntry))
	{
		UE_LOG(LogTemp, Warning, TEXT("UChatChannel::RestoreMessage — Failed to create ChatEntry widget"));
		return;
	}
	ChatEntry->Message = FString::Printf(TEXT("%s: %s"), *SenderName, *Message);
	if (ScrollBox)
	{
		ScrollBox->AddChild(ChatEntry);
		ScrollBox->ScrollToEnd();
	}
}
