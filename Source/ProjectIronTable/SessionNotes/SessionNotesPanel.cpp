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
#include "SessionNotesSave.h"

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

// Finds the matching FNoteRecord by ChannelID and updates its DisplayName in USessionNotesSave when the user renames a tab.
void USessionNotesPanel::OnChannelRenamed(UBaseChannelTab* Tab, const FString& NewName, const FString& ParticipantsKey)
{
	USessionInstance* SessionInstance = Cast<USessionInstance>(UGameplayStatics::GetGameInstance(this));
	if (!IsValid(SessionInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionNotesPanel::OnChannelRenamed — Failed to get SessionInstance; notes save will not be persisted"));
		return;
	}

	FString SlotName = UFunctionLibrary::GetNotesSaveSlotName(SessionInstance->GetPlayerID(), SessionInstance->GetSessionID());
	USessionNotesSave* SessionNotesSave = UFunctionLibrary::LoadSessionNotesSave(this);

	if (IsValid(SessionNotesSave))
	{
		USessionNotesChannel* NotesChannel = Cast<USessionNotesChannel>(Tab->GetChannel());
		if (IsValid(NotesChannel))
		{
			for (FNoteRecord& Record : SessionNotesSave->SessionNoteRecords)
			{
				if (Record.NoteID == NotesChannel->ChannelID)
				{
					Record.DisplayName = NewName;
					UGameplayStatics::SaveGameToSlot(SessionNotesSave, SlotName, 0);
					return;
				}
			}
		}
	}
}

// Iterates all channels, writes each channel's current document and timestamp into the matching save record, then saves to slot.
void USessionNotesPanel::OnChannelSwitched(UBaseChannel* Channel)
{
	USessionNotesSave* SessionNotesSave = UFunctionLibrary::LoadSessionNotesSave(this);
	if (!IsValid(SessionNotesSave))
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionNotesPanel::OnChannelSwitched — Failed to load notes save; channel documents will not be persisted"));
		return;
	}

	TArray<FNoteRecord>& Records = SessionNotesSave->SessionNoteRecords;

	for (UBaseChannel* LoopChannel : Channels)
	{
		USessionNotesChannel* NotesChannel = Cast<USessionNotesChannel>(LoopChannel);
		if (!IsValid(NotesChannel))
		{
			continue;
		}

		for (FNoteRecord& Record : Records)
		{
			if (Record.NoteID == NotesChannel->ChannelID)
			{
				Record.Content = NotesChannel->GetNotes()->GetDocument();
				Record.LastEdited = FDateTime::Now();
				break;
			}
		}
	}

	USessionInstance* SessionInstance = Cast<USessionInstance>(UGameplayStatics::GetGameInstance(this));
	if (!IsValid(SessionInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionNotesPanel::OnChannelSwitched — Failed to get SessionInstance; notes save will not be persisted"));
		return;
	}

	FString SlotName = UFunctionLibrary::GetNotesSaveSlotName(SessionInstance->GetPlayerID(), SessionInstance->GetSessionID());
	UGameplayStatics::SaveGameToSlot(SessionNotesSave, SlotName, 0);
}

// Calls base to create and wire the channel, then binds OnDocumentChanged to persist content and timestamp to USessionNotesSave on every edit.
UBaseChannel* USessionNotesPanel::CreateChannel(const TArray<FString>& Participants)
{
	UBaseChannel* Channel = Super::CreateChannel(Participants);
	USessionNotesChannel* NotesChannel = Cast<USessionNotesChannel>(Channel);
	CHECK_IF_VALID(NotesChannel, nullptr);

	NotesChannel->GetNotes()->GetOnDocumentChanged().AddLambda([this, NotesChannel]()
	{
		USessionNotesSave* SessionNotesSave = UFunctionLibrary::LoadSessionNotesSave(this);
		if (IsValid(SessionNotesSave))
		{
			for (FNoteRecord& Record : SessionNotesSave->SessionNoteRecords)
			{
				if (Record.NoteID == NotesChannel->ChannelID)
				{
					Record.Content = NotesChannel->GetNotes()->GetDocument();
					Record.LastEdited = FDateTime::Now();

					USessionInstance* SessionInstance = Cast<USessionInstance>(UGameplayStatics::GetGameInstance(this));
					if (!IsValid(SessionInstance))
					{
						UE_LOG(LogTemp, Warning, TEXT("USessionNotesPanel::CreateChannel — Failed to get SessionInstance; notes save will not be persisted"));
						break;
					}

					FString SlotName = UFunctionLibrary::GetNotesSaveSlotName(SessionInstance->GetPlayerID(), SessionInstance->GetSessionID());
					UGameplayStatics::SaveGameToSlot(SessionNotesSave, SlotName, 0);
					break;
				}
			}
		}
	});

	return Channel;
}

// Reuses Channels[0] for the first saved record, then calls Super::CreateChannel for each subsequent record and overwrites GUID, name, and document.
void USessionNotesPanel::RestoreChannels(USessionNotesSave* NotesSave)
{
	if (!IsValid(NotesSave))
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionNotesPanel::RestoreChannels — NotesSave is invalid"));
		return;
	}

	if (NotesSave->SessionNoteRecords.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionNotesPanel::RestoreChannels — No note records to restore"));
		return;
	}

	const TArray<FNoteRecord>& NoteRecords = NotesSave->SessionNoteRecords;
	UBaseChannel* Channel = Channels[0];
	USessionNotesChannel* NotesChannel = Cast<USessionNotesChannel>(Channel);
	CHECK_IF_VALID(NotesChannel, );

	NotesChannel->ChannelID = NoteRecords[0].NoteID;
	NotesChannel->DisplayName = NoteRecords[0].DisplayName;
	GetTabForChannel(Channel)->SetLabel(NotesChannel->DisplayName);
	NotesChannel->GetNotes()->SetDocument(NoteRecords[0].Content);

	for (int32 i = 1; i < NoteRecords.Num(); i++)
	{
		UBaseChannel* LoopChannel = Super::CreateChannel({});
		USessionNotesChannel* LoopNotesChannel = Cast<USessionNotesChannel>(LoopChannel);
		CHECK_IF_VALID(LoopNotesChannel, );

		LoopNotesChannel->ChannelID = NoteRecords[i].NoteID;
		LoopNotesChannel->DisplayName = NoteRecords[i].DisplayName;
		GetTabForChannel(LoopChannel)->SetLabel(LoopNotesChannel->DisplayName);
		LoopNotesChannel->GetNotes()->SetDocument(NoteRecords[i].Content);
	}
}
