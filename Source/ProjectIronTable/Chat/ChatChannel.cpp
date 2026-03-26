// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatChannel.h"
#include "ChatEntry.h"

void UChatChannel::NativeConstruct()
{
	Super::NativeConstruct();
}

void UChatChannel::AddChatMessage(const FString& Message)
{
	if (!ChatEntryClass) return;
	UChatEntry* ChatEntry = CreateWidget<UChatEntry>(GetWorld(), ChatEntryClass);
	ChatEntry->Message = Message;
	ScrollBox->AddChild(ChatEntry);
	ScrollBox->ScrollToEnd();
}

void UChatChannel::SetChatEntryClass(TSubclassOf<UChatEntry> EntryClass)
{
	ChatEntryClass = EntryClass;
}
