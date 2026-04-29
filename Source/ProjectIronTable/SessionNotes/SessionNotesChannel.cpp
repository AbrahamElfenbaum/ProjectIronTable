// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionNotesChannel.h"

#include "Components/ScrollBox.h"

#include "EditableRichText.h"

// Scrolls the channel's scroll box to the end when the notes content changes.
void USessionNotesChannel::OnNotesChanged()
{
	ScrollToEnd();
}

// Calls Super, generates a unique ChannelID, then binds the document-changed delegate to auto-scroll on content updates.
void USessionNotesChannel::NativeConstruct()
{
	Super::NativeConstruct();
	ChannelID = FGuid::NewGuid();

	if (Notes)
	{
		Notes->GetOnDocumentChanged().AddUObject(this, &USessionNotesChannel::OnNotesChanged);
	}
}

// Scrolls the channel's scroll box to the bottom; logs a warning if ScrollBox is not bound.
void USessionNotesChannel::ScrollToEnd()
{
	if (!ScrollBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionNotesChannel::ScrollToEnd — ScrollBox is null"));
		return;
	}
	ScrollBox->ScrollToEnd();
}

// Returns the Notes rich text widget for this channel.
UEditableRichText* USessionNotesChannel::GetNotes()
{
	return Notes;
}
