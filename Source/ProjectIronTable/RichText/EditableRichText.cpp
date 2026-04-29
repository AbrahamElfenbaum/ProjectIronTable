// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "EditableRichText.h"

// Creates the SRichTextEditor Slate widget and returns it for UMG to display.
TSharedRef<SWidget> UEditableRichText::RebuildWidget()
{
	RichTextEditor = SNew(SRichTextEditor);
	return RichTextEditor.ToSharedRef();
}

// Releases the Slate widget and resets the shared pointer to free memory.
void UEditableRichText::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	RichTextEditor.Reset();
}

// Passes bold toggle through to the Slate editor if it is valid.
void UEditableRichText::ToggleBold(bool bEnable)
{
	if (RichTextEditor)
	{
		RichTextEditor->ToggleBold(bEnable);
	}
}

// Passes italic toggle through to the Slate editor if it is valid.
void UEditableRichText::ToggleItalic(bool bEnable)
{
	if (RichTextEditor)
	{
		RichTextEditor->ToggleItalic(bEnable);
	}
}

// Passes underline toggle through to the Slate editor if it is valid.
void UEditableRichText::ToggleUnderline(bool bEnable)
{
	if (RichTextEditor)
	{
		RichTextEditor->ToggleUnderline(bEnable);
	}
}

// Passes strikethrough toggle through to the Slate editor if it is valid.
void UEditableRichText::ToggleStrikethrough(bool bEnable)
{
	if (RichTextEditor)
	{
		RichTextEditor->ToggleStrikethrough(bEnable);
	}
}

// Returns the editor's current document, or a default-constructed document if the editor is not yet built.
FRichTextDocument UEditableRichText::GetDocument() const
{
	if (RichTextEditor)
	{
		return RichTextEditor->GetDocument();
	}
	return FRichTextDocument();
}

// Passes the document to the Slate editor for display and editing if it is valid.
void UEditableRichText::SetDocument(const FRichTextDocument& InDocument)
{
	if (RichTextEditor)
	{
		RichTextEditor->SetDocument(InDocument);
	}
}

// Returns a reference to the Slate editor's OnDocumentChanged delegate so callers can bind without going through the Slate layer directly.
FOnDocumentChanged& UEditableRichText::GetOnDocumentChanged()
{
	return RichTextEditor->OnDocumentChanged;
}
