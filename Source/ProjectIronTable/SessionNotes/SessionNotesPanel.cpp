// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionNotesPanel.h"

#include "Components/MultiLineEditableText.h"

#include "BaseChannel.h"
#include "SessionNotesChannel.h"

// Creates a new notes channel, assigns a GUID, and wires it via Super. Stub — pending full implementation.
UBaseChannel* USessionNotesPanel::CreateChannel(const TArray<FString>& Participants)
{
	return nullptr;
}

// Returns a display label for the new notes tab. Stub — pending numbering logic.
FString USessionNotesPanel::CreateTabLabel(const TArray<FString>& Participants) const
{
	return FString();
}

// Persists the newly created tab's GUID and label to USessionSave. Stub — pending implementation.
void USessionNotesPanel::SaveCreatedTab()
{
}

// Updates the saved tab label in USessionSave on rename. Stub — pending implementation.
void USessionNotesPanel::OnChannelRenamed(UBaseChannelTab* Tab, const FString& NewName, const FString& ParticipantsKey)
{
}

// Loads saved content for the switched-to channel. Stub — pending implementation.
void USessionNotesPanel::OnChannelSwitched(UBaseChannel* Channel)
{
}

// Calls Super, then binds the text changed delegate to auto-scroll on new content.
void USessionNotesPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(NotesText))
	{
		NotesText->OnTextChanged.AddDynamic(this, &USessionNotesPanel::OnNotesTextChanged);
	}
}

// Scrolls the notes scroll box to the end when the text content grows.
void USessionNotesPanel::OnNotesTextChanged(const FText& Text)
{
	USessionNotesChannel* NotesChannel = Cast<USessionNotesChannel>(ActiveChannel);
	if (IsValid(NotesChannel))
	{
		NotesChannel->ScrollToEnd();
	}
}
