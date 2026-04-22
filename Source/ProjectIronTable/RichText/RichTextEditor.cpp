// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "RichTextEditor.h"

// Initializes default state and sets up a placeholder child slot. Full layout built when toolbar and text area are added.
void SRichTextEditor::Construct(const FArguments& InArgs)
{
	ChildSlot[SNew(SBox)];
}

// Sets the bold flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleBold(bool bEnable)
{
	ActiveFormat.bIsBold = bEnable;
}

// Sets the italic flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleItalic(bool bEnable)
{
	ActiveFormat.bIsItalic = bEnable;
}

// Sets the underline flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleUnderline(bool bEnable)
{
	ActiveFormat.bIsUnderline = bEnable;
}

// Sets the strikethrough flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleStrikethrough(bool bEnable)
{
	ActiveFormat.bIsStrikethrough = bEnable;
}

// Returns the document's current run list for serialization.
FRichTextDocument SRichTextEditor::GetDocument() const
{
	return Document;
}

// Replaces the current document with the given one, restoring saved content.
void SRichTextEditor::SetDocument(const FRichTextDocument& InDocument)
{
	Document = InDocument;
}