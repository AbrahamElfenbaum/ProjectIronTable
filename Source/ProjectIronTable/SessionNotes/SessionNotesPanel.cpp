// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionNotesPanel.h"

#include "Kismet/GameplayStatics.h"

#include "BaseChannel.h"
#include "BaseChannelTab.h"
#include "EditableRichText.h"
#include "FunctionLibrary.h"
#include "MacroLibrary.h"
#include "SessionInstance.h"
#include "SessionNotesChannel.h"
#include "SessionSave.h"

// Returns "Notes" for the first tab, or "Notes N" where N is Channels.Num() + 1 for subsequent tabs.
FString USessionNotesPanel::CreateTabLabel(const TArray<FString>& Participants) const
{
	if (Channels.IsEmpty())
	{
		return TEXT("Notes");
	}
	return FString::Printf(TEXT("Notes %d"), Channels.Num() + 1);
}

// No-op — save is handled directly in CreateChannel.
void USessionNotesPanel::SaveCreatedTab()
{
}

// Casts the tab's channel to USessionNotesChannel, then updates NotesTabNames in USessionSave with the new name.
void USessionNotesPanel::OnChannelRenamed(UBaseChannelTab* Tab, const FString& NewName, const FString& ParticipantsKey)
{
	USessionSave* SessionSave = UFunctionLibrary::LoadSessionSave(this);
	if (IsValid(SessionSave))
	{
		USessionNotesChannel* NotesChannel = Cast<USessionNotesChannel>(Tab->GetChannel());
		if (IsValid(NotesChannel))
		{
			SessionSave->NotesTabNames[NotesChannel->ChannelID] = NewName;
			UGameplayStatics::SaveGameToSlot(SessionSave, UFunctionLibrary::GetSessionSaveSlotName(GetGameInstance<USessionInstance>()), 0);
		}
	}
}

// No-op — the channel widget retains its in-memory document on switch.
void USessionNotesPanel::OnChannelSwitched(UBaseChannel* Channel)
{
}

// Calls base to create and wire the channel, then saves its GUID and initial document to USessionSave.
UBaseChannel* USessionNotesPanel::CreateChannel(const TArray<FString>& Participants)
{
	UBaseChannel* Channel = Super::CreateChannel(Participants);
	USessionNotesChannel* NotesChannel = Cast<USessionNotesChannel>(Channel);
	CHECK_IF_VALID(NotesChannel, nullptr);

	USessionSave* SessionSave = UFunctionLibrary::LoadSessionSave(this);
	if (IsValid(SessionSave))
	{
		if (!SessionSave->NotesTabNames.Find(NotesChannel->ChannelID))
		{
			SessionSave->NotesTabNames.Add(NotesChannel->ChannelID, NotesChannel->DisplayName);
			SessionSave->NotesTabContent.Add(NotesChannel->ChannelID, NotesChannel->GetNotes()->GetDocument());
		}
		UGameplayStatics::SaveGameToSlot(SessionSave, UFunctionLibrary::GetSessionSaveSlotName(GetGameInstance<USessionInstance>()), 0);
	}

	return Channel;
}
