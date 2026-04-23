// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "RichTextEditorWidget.h"

// Passes bold toggle through to the Slate editor if it is valid.
void URichTextEditorWidget::ToggleBold(bool bEnable)
{
	if (RichTextEditor)
	{
		RichTextEditor->ToggleBold(bEnable);
	}
}

// Passes italic toggle through to the Slate editor if it is valid.
void URichTextEditorWidget::ToggleItalic(bool bEnable)
{
	if (RichTextEditor)
	{
		RichTextEditor->ToggleItalic(bEnable);
	}
}

// Passes underline toggle through to the Slate editor if it is valid.
void URichTextEditorWidget::ToggleUnderline(bool bEnable)
{
	if (RichTextEditor)
	{
		RichTextEditor->ToggleUnderline(bEnable);
	}
}

// Passes strikethrough toggle through to the Slate editor if it is valid.
void URichTextEditorWidget::ToggleStrikethrough(bool bEnable)
{
	if (RichTextEditor)
	{
		RichTextEditor->ToggleStrikethrough(bEnable);
	}
}

// Returns the editor's current document, or a default-constructed document if the editor is not yet built.
FRichTextDocument URichTextEditorWidget::GetDocument() const
{
	if (RichTextEditor)
	{
		return RichTextEditor->GetDocument();
	}
	return FRichTextDocument();
}

// Passes the document to the Slate editor for display and editing if it is valid.
void URichTextEditorWidget::SetDocument(const FRichTextDocument& InDocument)
{
	if (RichTextEditor)
	{
		RichTextEditor->SetDocument(InDocument);
	}
}

// Creates the SRichTextEditor Slate widget and returns it for UMG to display.
TSharedRef<SWidget> URichTextEditorWidget::RebuildWidget()
{
	RichTextEditor = SNew(SRichTextEditor);
	return RichTextEditor.ToSharedRef();
}

// Releases the Slate widget and resets the shared pointer to free memory.
void URichTextEditorWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	RichTextEditor.Reset();
}