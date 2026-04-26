// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "RichTextEditor.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "RichTextArea.h"
#include "RichTextRun.h"
#include "Fonts/FontMeasure.h"

// Builds the editor layout: a toolbar row with four format checkboxes above an empty content area for painted text.
void SRichTextEditor::Construct(const FArguments& InArgs)
{
	ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()[MakeFormatCheckbox(BoldCheckbox, [this](bool b) { ToggleBold(b); }, TEXT("B"))]
						+ SHorizontalBox::Slot().AutoWidth()[MakeFormatCheckbox(ItalicCheckbox, [this](bool b) { ToggleItalic(b); }, TEXT("I"))]
						+ SHorizontalBox::Slot().AutoWidth()[MakeFormatCheckbox(UnderlineCheckbox, [this](bool b) { ToggleUnderline(b); }, TEXT("U"))]
						+ SHorizontalBox::Slot().AutoWidth()[MakeFormatCheckbox(StrikethroughCheckbox, [this](bool b) { ToggleStrikethrough(b); }, TEXT("S"))]
				]
			+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SNew(SRichTextArea).Document(&Document).CursorPosition(&CursorPosition)
				]
		];

#if 1
	Document.Runs.Add(FRichTextRun(TEXT(""), FCoreStyle::GetDefaultFontStyle("Regular", 12)));
	ActiveFormat.FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", 12);
	//CursorPosition = Document.Runs[0].Text.Len();
#endif // 1

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

// Returns the current document.
FRichTextDocument SRichTextEditor::GetDocument() const
{
	return Document;
}

// Replaces the current document and resets the cursor to the start.
void SRichTextEditor::SetDocument(const FRichTextDocument& InDocument)
{
	Document = InDocument;
	CursorPosition = 0;
}

// Always returns true so the widget can receive keyboard input.
bool SRichTextEditor::SupportsKeyboardFocus() const
{
	return true;
}

// Inserts the typed character into the correct run at CursorPosition and advances the cursor.
FReply SRichTextEditor::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent)
{
	TCHAR Character = InCharacterEvent.GetCharacter();

	if (Character < 32)
	{
		return FReply::Unhandled();
	}

	int32 RunStart = 0;
	int32 RunIndex = FindRunAtIndex(CursorPosition, RunStart);

	if (FormatsMatch(Document.Runs[RunIndex], ActiveFormat))
	{
		Document.Runs[RunIndex].Text.InsertAt(CursorPosition - RunStart, Character);
	}
	else
	{
		FRichTextRun& FoundRun = Document.Runs[RunIndex];

		FRichTextRun LeftRun = FoundRun;
		LeftRun.Text = FoundRun.Text.Left(CursorPosition - RunStart);

		FRichTextRun MiddleRun = ActiveFormat;
		MiddleRun.Text = FString(1, &Character);

		FRichTextRun RightRun = FoundRun;
		RightRun.Text = FoundRun.Text.Mid(CursorPosition - RunStart);

		Document.Runs.RemoveAt(RunIndex);

		Document.Runs.Insert(LeftRun, RunIndex);
		Document.Runs.Insert(MiddleRun, RunIndex + 1);
		Document.Runs.Insert(RightRun, RunIndex + 2);
	}

	CursorPosition++;
	SyncActiveFormat();
	return FReply::Handled();
}

// Handles cursor movement, deletion, and format shortcut keys.
FReply SRichTextEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	FKey DownKey = InKeyEvent.GetKey();
	bool bControlDown = InKeyEvent.IsControlDown();

	TArray<FString> Lines;
	Document.GetFullText().ParseIntoArray(Lines, TEXT("\n"), false);
	float TabSpace = SRichTextArea::MeasureText(TEXT("    "), Document.Runs[0].FontInfo, MyGeometry.Scale);
	FVector2f CursorPos = SRichTextArea::GetCursorPosition(Document, CursorPosition, TabSpace, MyGeometry.Scale);

	if (DownKey == EKeys::BackSpace)
	{
		if (CursorPosition != 0)
		{
			OnBackspaceOrDeletePressed(CursorPosition - 1);
			CursorPosition = FMath::Max(0, CursorPosition - 1);
			SyncActiveFormat();
			return FReply::Handled();
		}

	}
	else if (DownKey == EKeys::Delete)
	{
		if (CursorPosition != Document.GetFullText().Len())
		{
			OnBackspaceOrDeletePressed(CursorPosition);
			SyncActiveFormat();
			return FReply::Handled();
		}
	}
	else if (DownKey == EKeys::Left)
	{
		CursorPosition = FMath::Max(0, CursorPosition - 1);
		SyncActiveFormat();
		return FReply::Handled();
	}
	else if (DownKey == EKeys::Right)
	{
		CursorPosition = FMath::Min(Document.GetFullText().Len(), CursorPosition + 1);
		SyncActiveFormat();
		return FReply::Handled();
	}
	else if (DownKey == EKeys::Up)
	{
		return OnUpOrDownPressed(Lines, CursorPos, MyGeometry.Scale, true);
	}
	else if (DownKey == EKeys::Down)
	{
		return OnUpOrDownPressed(Lines, CursorPos, MyGeometry.Scale, false);
	}
	else if (DownKey == EKeys::Home)
	{
		CursorPosition = 0;
		SyncActiveFormat();
		return FReply::Handled();
	}
	else if (DownKey == EKeys::End)
	{
		CursorPosition = Document.GetFullText().Len();
		SyncActiveFormat();
		return FReply::Handled();
	}
	else if (DownKey == EKeys::Enter)
	{
		return DrawSpecialCharacter('\n');
	}
	else if (DownKey == EKeys::Tab)
	{
		return DrawSpecialCharacter('\t');
	}
	else if (bControlDown && DownKey == EKeys::A)
	{
		return FReply::Unhandled();
	}
	else if (bControlDown && DownKey == EKeys::Z)
	{
		return FReply::Unhandled();
	}
	else if (bControlDown && DownKey == EKeys::Y)
	{
		return FReply::Unhandled();
	}
	else if (bControlDown && DownKey == EKeys::B)
	{
		ActiveFormat.bIsBold = !ActiveFormat.bIsBold;
		BoldCheckbox->SetIsChecked(ActiveFormat.bIsBold ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		return FReply::Handled();
	}
	else if (bControlDown && DownKey == EKeys::I)
	{
		ActiveFormat.bIsItalic = !ActiveFormat.bIsItalic;
		ItalicCheckbox->SetIsChecked(ActiveFormat.bIsItalic ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		return FReply::Handled();
	}
	else if (bControlDown && DownKey == EKeys::U)
	{
		ActiveFormat.bIsUnderline = !ActiveFormat.bIsUnderline;
		UnderlineCheckbox->SetIsChecked(ActiveFormat.bIsUnderline ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		return FReply::Handled();
	}

	return FReply::Handled();
}

// Requests keyboard focus when the widget is clicked.
FReply SRichTextEditor::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	return FReply::Handled();
}

// Walks Document.Runs, accumulating character offsets, and returns the index of the run whose range spans CharIndex.
int32 SRichTextEditor::FindRunAtIndex(int32 CharIndex, int32& OutRunStart) const
{
	int32 CurrentIndex = 0;
	int32 RunIndex = 0;
	for (const FRichTextRun& Run : Document.Runs)
	{
		if (CharIndex <= CurrentIndex + Run.Text.Len())
		{
			OutRunStart = CurrentIndex;
			return RunIndex;
		}
		RunIndex++;
		CurrentIndex += Run.Text.Len();
	}

	OutRunStart = CurrentIndex;
	return Document.Runs.Num() - 1;
}

// Creates a checkbox wired to the given callback and labeled with the given string, assigning the shared pointer for later access.
TSharedRef<SWidget> SRichTextEditor::MakeFormatCheckbox(TSharedPtr<SCheckBox>& Checkbox, TFunction<void(bool)> Callback, const TCHAR* Label)
{
	return SAssignNew(Checkbox, SCheckBox)
		.OnCheckStateChanged(FOnCheckStateChanged::CreateLambda([Callback](ECheckBoxState State)
			{
				Callback(State == ECheckBoxState::Checked);
			}))
		[SNew(STextBlock).Text(FText::FromString(FString(Label)))];
}

// Returns true if both runs share the same bold, italic, underline, strikethrough flags and font info.
bool SRichTextEditor::FormatsMatch(const FRichTextRun& A, const FRichTextRun& B) const
{
	if (A.bIsBold != B.bIsBold)
	{
		return false;
	}

	if (A.bIsItalic != B.bIsItalic)
	{
		return false;
	}

	if (A.bIsUnderline != B.bIsUnderline)
	{
		return false;
	}

	if (A.bIsStrikethrough != B.bIsStrikethrough)
	{
		return false;
	}

	if (A.FontInfo != B.FontInfo)
	{
		return false;
	}

	return true;
}

// Inserts a non-printable character at CursorPosition via the same run-walk as OnKeyChar, then advances the cursor.
FReply SRichTextEditor::DrawSpecialCharacter(TCHAR SpecialCharacter)
{
	int32 RunStart = 0;
	int32 RunIndex = FindRunAtIndex(CursorPosition, RunStart);
	Document.Runs[RunIndex].Text.InsertAt(CursorPosition - RunStart, SpecialCharacter);
	CursorPosition++;

	SyncActiveFormat();
	return FReply::Handled();
}

// Finds the run at CursorPos, redirects to the next run if CursorPos is on a boundary, then removes the character at the resolved local offset.
void SRichTextEditor::OnBackspaceOrDeletePressed(int32 CursorPos)
{
	int32 RunStart = 0;
	int32 Index = FindRunAtIndex(CursorPos, RunStart);
	int32 LocalOffset = CursorPos - RunStart;
	if (LocalOffset >= Document.Runs[Index].Text.Len())
	{
		RunStart += Document.Runs[Index].Text.Len();
		Index++;
		LocalOffset = 0;
	}
	Document.Runs[Index].Text.RemoveAt(LocalOffset);

	if (Document.Runs[Index].Text.IsEmpty() && Document.Runs.Num() > 1)
	{
		Document.Runs.RemoveAt(Index);
	}
}

// Moves the cursor up or down one line, finding the character on the target line closest to the cursor's X pixel position.
FReply SRichTextEditor::OnUpOrDownPressed(const TArray<FString>& Lines, FVector2f CursorPos, float Scale, bool bUp)
{
	uint16 LineHeight = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
		->GetMaxCharacterHeight(Document.Runs[0].FontInfo, Scale);

	int32 CurrentLine = CursorPos.Y / LineHeight;
	int32 TargetLine = CurrentLine + (bUp ? -1 : 1);

	if ((bUp && TargetLine < 0) || (!bUp && TargetLine >= Lines.Num()))
	{
		return FReply::Handled();
	}

	int32 CharInLine = 0;
	for (int32 i = 0; i < Lines[TargetLine].Len(); i++)
	{
		float CharWidth = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
			->Measure(Lines[TargetLine].Left(i + 1), Document.Runs[0].FontInfo, Scale).X / Scale;
		if (CharWidth > CursorPos.X)
		{
			break;
		}
		CharInLine = i + 1;
	}

	int32 NewCursorPosition = 0;
	for (int32 i = 0; i < TargetLine; i++)
	{
		NewCursorPosition += Lines[i].Len() + 1;
	}
	NewCursorPosition += CharInLine;
	CursorPosition = NewCursorPosition;

	SyncActiveFormat();
	return FReply::Handled();
}

// Copies the format flags of the run under CursorPosition into ActiveFormat so newly typed text inherits the correct formatting.
void SRichTextEditor::SyncActiveFormat()
{
	int32 RunStart = 0;
	int32 RunIndex = FindRunAtIndex(CursorPosition, RunStart);
	ActiveFormat = Document.Runs[RunIndex];


	BoldCheckbox->SetIsChecked(ActiveFormat.bIsBold ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	ItalicCheckbox->SetIsChecked(ActiveFormat.bIsItalic ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	UnderlineCheckbox->SetIsChecked(ActiveFormat.bIsUnderline ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	StrikethroughCheckbox->SetIsChecked(ActiveFormat.bIsStrikethrough ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
}
