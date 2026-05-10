// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionNotesChannel.h"

#include "Components/CheckBox.h"
#include "Components/ScrollBox.h"

#include "EditableRichText.h"

// Scrolls the channel's scroll box to the end when the notes document changes.
void USessionNotesChannel::OnNotesChanged()
{
	ScrollToEnd();
}

// Forwards the bold state to the rich text editor, guarded against feedback from programmatic checkbox updates.
void USessionNotesChannel::OnBoldCheckStateChanged(bool bIsChecked)
{
	if (bIsSyncing)
	{
		return;
	}

	Notes->ToggleBold(bIsChecked);
}

// Forwards the italic state to the rich text editor, guarded against feedback from programmatic checkbox updates.
void USessionNotesChannel::OnItalicCheckStateChanged(bool bIsChecked)
{
	if (bIsSyncing)
	{
		return;
	}

	Notes->ToggleItalic(bIsChecked);
}

// Forwards the underline state to the rich text editor, guarded against feedback from programmatic checkbox updates.
void USessionNotesChannel::OnUnderlineCheckStateChanged(bool bIsChecked)
{
	if (bIsSyncing)
	{
		return;
	}

	Notes->ToggleUnderline(bIsChecked);
}

// Forwards the strikethrough state to the rich text editor, guarded against feedback from programmatic checkbox updates.
void USessionNotesChannel::OnStrikethroughCheckStateChanged(bool bIsChecked)
{
	if (bIsSyncing)
	{
		return;
	}

	Notes->ToggleStrikethrough(bIsChecked);
}

// Sets bIsSyncing and updates all four toolbar checkboxes to reflect the current editor format state without triggering feedback.
void USessionNotesChannel::OnFormatChanged(bool bBold, bool bItalic, bool bUnderline, bool bStrikethrough)
{
	bIsSyncing = true;

	BoldCheckBox->SetIsChecked(bBold);
	ItalicCheckBox->SetIsChecked(bItalic);
	UnderlineCheckBox->SetIsChecked(bUnderline);
	StrikethroughCheckBox->SetIsChecked(bStrikethrough);

	bIsSyncing = false;
}

// Calls Super, generates a unique ChannelID, then binds the document-changed delegate to auto-scroll on content updates.
void USessionNotesChannel::NativeConstruct()
{
	Super::NativeConstruct();
	ChannelID = FGuid::NewGuid();

	if (Notes)
	{
		Notes->GetOnDocumentChanged().AddUObject(this, &USessionNotesChannel::OnNotesChanged);
		Notes->GetOnFormatStateChanged().AddUObject(this, &USessionNotesChannel::OnFormatChanged);
	}

	if (BoldCheckBox)
	{
		BoldCheckBox->OnCheckStateChanged.AddDynamic(this, &USessionNotesChannel::OnBoldCheckStateChanged);
	}

	if (ItalicCheckBox)
	{
		ItalicCheckBox->OnCheckStateChanged.AddDynamic(this, &USessionNotesChannel::OnItalicCheckStateChanged);
	}

	if (UnderlineCheckBox)
	{
		UnderlineCheckBox->OnCheckStateChanged.AddDynamic(this, &USessionNotesChannel::OnUnderlineCheckStateChanged);
	}

	if (StrikethroughCheckBox)
	{
		StrikethroughCheckBox->OnCheckStateChanged.AddDynamic(this, &USessionNotesChannel::OnStrikethroughCheckStateChanged);
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
