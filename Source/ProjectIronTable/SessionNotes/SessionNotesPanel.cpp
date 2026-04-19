// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionNotesPanel.h"

#include "Components/ScrollBox.h"
#include "Components/MultiLineEditableText.h"

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
	if (IsValid(NotesScroll))
	{
		NotesScroll->ScrollToEnd();
	}
}
