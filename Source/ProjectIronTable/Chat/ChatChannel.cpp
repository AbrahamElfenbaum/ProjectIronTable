// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatChannel.h"
#include "ChatEntry.h"

// Called when the channel widget is constructed.
void UChatChannel::NativeConstruct()
{
	Super::NativeConstruct();
}

// Creates a UChatEntry widget with the given message, appends it to the scroll box, and scrolls to the bottom.
void UChatChannel::AddChatMessage(const FString& Message)
{
	if (!ChatEntryClass) return;
	UChatEntry* ChatEntry = CreateWidget<UChatEntry>(GetWorld(), ChatEntryClass);
	ChatEntry->Message = Message;
	ScrollBox->AddChild(ChatEntry);
	ScrollBox->ScrollToEnd();
}

// Sets the entry class used when creating new message widgets.
void UChatChannel::SetChatEntryClass(TSubclassOf<UChatEntry> EntryClass)
{
	ChatEntryClass = EntryClass;
}

// Adjusts the scroll offset by ScrollMultiplier in the requested direction, clamped to valid range.
void UChatChannel::Scroll(bool bUp)
{
	int32 ScrollDirection = bUp ? 1 : -1;
	ScrollBox->SetScrollOffset(
		FMath::Clamp(
			ScrollBox->GetScrollOffset() + (ScrollMultiplier * ScrollDirection),
			0.0f,
			ScrollBox->GetScrollOffsetOfEnd()));
}
