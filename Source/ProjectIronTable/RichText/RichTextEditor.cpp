// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "RichTextEditor.h"

#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"

// Builds the editor layout: a toolbar row with four format checkboxes above a scrollable multiline text area.
void SRichTextEditor::Construct(const FArguments& InArgs)
{
	ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()[MakeFormatCheckbox(BoldCheckbox,           [this](bool b){ ToggleBold(b); },          TEXT("B"))]
						+ SHorizontalBox::Slot().AutoWidth()[MakeFormatCheckbox(ItalicCheckbox,         [this](bool b){ ToggleItalic(b); },        TEXT("I"))]
						+ SHorizontalBox::Slot().AutoWidth()[MakeFormatCheckbox(UnderlineCheckbox,      [this](bool b){ ToggleUnderline(b); },     TEXT("U"))]
						+ SHorizontalBox::Slot().AutoWidth()[MakeFormatCheckbox(StrikethroughCheckbox,  [this](bool b){ ToggleStrikethrough(b); }, TEXT("S"))]
				]
			+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(TextArea, SMultiLineEditableText)
						]
				]
		];
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

// Returns the current text as a single-run document. Returns an empty document if the text area is not yet constructed.
FRichTextDocument SRichTextEditor::GetDocument() const
{
	if (!TextArea.IsValid())
	{
		return FRichTextDocument{};
	}
	FRichTextRun Run;
	Run.Text = TextArea->GetText().ToString();
	FRichTextDocument Result;
	Result.Runs.Add(Run);
	return Result;
}

// Pushes the first run's text into the text area. No-ops if the text area is not yet constructed or the document has no runs.
void SRichTextEditor::SetDocument(const FRichTextDocument& InDocument)
{
	if (!TextArea.IsValid() || InDocument.Runs.IsEmpty())
	{
		return;
	}
	TextArea->SetText(FText::FromString(InDocument.Runs[0].Text));
}

// Creates a checkbox wired to the given callback and labeled with the given string, assigning the shared pointer for later access.
TSharedRef<SWidget> SRichTextEditor::MakeFormatCheckbox(TSharedPtr<SCheckBox>& Checkbox, TFunction<void(bool)> Callback, const TCHAR* Label)
{
	return SAssignNew(Checkbox, SCheckBox)
		.OnCheckStateChanged(FOnCheckStateChanged::CreateLambda([Callback](ECheckBoxState State)
		{
			Callback(State == ECheckBoxState::Checked);
		}))
		[ SNew(STextBlock).Text(FText::FromString(FString(Label))) ];
}
