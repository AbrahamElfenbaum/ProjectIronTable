// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionNotesPanel.h"

#include "Components/MultiLineEditableText.h"
#include "BaseChannel.h"
#include "SessionNotesChannel.h"

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

UBaseChannel* USessionNotesPanel::CreateChannel(const TArray<FString>& Participants)
{
	return nullptr;
}

FString USessionNotesPanel::CreateTabLabel(const TArray<FString>& Participants) const
{
	return FString();
}

void USessionNotesPanel::SaveCreatedTab()
{
}

void USessionNotesPanel::OnChannelRenamed(UBaseChannelTab* Tab, const FString& NewName, const FString& ParticipantsKey)
{
}

void USessionNotesPanel::OnChannelSwitched(UBaseChannel* Channel)
{
}
