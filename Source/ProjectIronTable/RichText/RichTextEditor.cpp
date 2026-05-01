// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "RichTextEditor.h"

#include "Fonts/FontMeasure.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#include "RichTextArea.h"
#include "RichTextRun.h"

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
		.IsFocusable(false)
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
void SRichTextEditor::DrawSpecialCharacter(TCHAR SpecialCharacter)
{
	int32 RunStart = 0;
	int32 RunIndex = FindRunAtIndex(CursorPosition, RunStart);
	Document.Runs[RunIndex].Text.InsertAt(CursorPosition - RunStart, SpecialCharacter);
	CursorPosition++;
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
	PruneRuns();
}

// Moves the cursor up or down one line, finding the character on the target line closest to the cursor's X pixel position.
void SRichTextEditor::OnUpOrDownPressed(const TArray<FString>& Lines, FVector2f CursorPos, float Scale, bool bUp)
{
	float LineHeight = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
		->GetMaxCharacterHeight(Document.Runs[0].FontInfo, Scale);

	int32 CurrentLine = CursorPos.Y / LineHeight;
	int32 TargetLine = CurrentLine + (bUp ? -1 : 1);

	if (!((bUp && TargetLine < 0) || (!bUp && TargetLine >= Lines.Num())))
	{
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
	}
}

// Copies the format flags of the run under CursorPosition into ActiveFormat so newly typed text inherits the correct formatting.
void SRichTextEditor::SyncActiveFormat()
{
	int32 RunStart = 0;
	int32 RunIndex = FindRunAtIndex(CursorPosition, RunStart);
	ActiveFormat = Document.Runs[RunIndex];

	bIsSyncing = true;

	BoldCheckbox->SetIsChecked(ActiveFormat.bIsBold ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	ItalicCheckbox->SetIsChecked(ActiveFormat.bIsItalic ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	UnderlineCheckbox->SetIsChecked(ActiveFormat.bIsUnderline ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	StrikethroughCheckbox->SetIsChecked(ActiveFormat.bIsStrikethrough ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);

	bIsSyncing = false;
}

// Removes all empty runs, merges adjacent runs with identical format, and re-adds a blank default run if none remain.
void SRichTextEditor::PruneRuns()
{
	Document.Runs.RemoveAll([](const FRichTextRun& Run)
		{
			return Run.Text.IsEmpty();
		});

	if (Document.Runs.IsEmpty())
	{
		ActiveFormat.Text.Empty();
		Document.Runs.Add(ActiveFormat);
	}

	if (Document.Runs.Num() > 1)
	{
		TArray<FRichTextRun>& Runs = Document.Runs;
		for (int32 i = 0; i < Runs.Num() - 1;)
		{
			if (FormatsMatch(Runs[i], Runs[i + 1]))
			{
				Runs[i].Text += Runs[i + 1].Text;
				Runs.RemoveAt(i + 1);
			}
			else
			{
				i++;
			}
		}
	}
}

// Converts the mouse screen-space position to a character index using TextAreaRef's local geometry so toolbar height does not bias the Y coordinate.
int32 SRichTextEditor::GetAreaCursorPosition(const FPointerEvent& MouseEvent)
{
	FVector2f LocalMousePos = FVector2f(TextAreaRef->GetTickSpaceGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	return SRichTextArea::HitTest(LocalMousePos, Document, TextAreaRef->GetTickSpaceGeometry().Scale);
}

// Walks Document.Runs from the run containing SelectionMin to the run containing SelectionMax, clipping each run's text to the selection boundaries and copying all format flags.
TArray<FRichTextRun> SRichTextEditor::GetSelectedRange()
{
	int32 SelectionMin = GetSelectionMin();
	int32 SelectionMax = GetSelectionMax();

	int32 StartRunStartCharOffset = 0;
	int32 EndRunStartCharOffset = 0;

	int32 StartRunIndex = FindRunAtIndex(SelectionMin, StartRunStartCharOffset);
	int32 EndRunIndex = FindRunAtIndex(SelectionMax, EndRunStartCharOffset);

	TArray<FRichTextRun> SelectedRuns;

	int32 RunStart = StartRunStartCharOffset;
	for (int i = StartRunIndex; i <= EndRunIndex; i++)
	{
		FRichTextRun CurRun = Document.Runs[i];
		CurRun.Text = Document.Runs[i].Text.Mid((FMath::Max(RunStart, SelectionMin) - RunStart),
												 FMath::Min(RunStart + Document.Runs[i].Text.Len(), SelectionMax) -
												 FMath::Max(RunStart, SelectionMin));
		SelectedRuns.Add(CurRun);
		RunStart += Document.Runs[i].Text.Len();
	}

	return SelectedRuns;
}

// Sets SelectionAnchor on the first Shift+move to capture the pre-move position, or clears it to -1 on a bare move to collapse the selection.
void SRichTextEditor::HandleSelectionOnMove(bool bShiftDown)
{
	if(bShiftDown)
	{
		if (SelectionAnchor == -1)
		{
			SelectionAnchor = CursorPosition;
		}
		GetSelectedRange();
	}
	else
	{
		SelectionAnchor = -1;
	}
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
		PruneRuns();
	}

	CursorPosition++;
	SyncActiveFormat();
	OnDocumentChanged.Broadcast();
	return FReply::Handled();
}

// Handles cursor movement, deletion, and format shortcut keys.
FReply SRichTextEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	bool bHandled = false;
	FKey DownKey = InKeyEvent.GetKey();
	bool bControlDown = InKeyEvent.IsControlDown();
	bool bShiftDown = InKeyEvent.IsLeftShiftDown() || InKeyEvent.IsRightShiftDown();

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
			bHandled = true;
		}
	}
	else if (DownKey == EKeys::Delete)
	{
		if (CursorPosition != Document.GetFullText().Len())
		{
			OnBackspaceOrDeletePressed(CursorPosition);
			bHandled = true;
		}
	}
	else if (DownKey == EKeys::Left)
	{
		HandleSelectionOnMove(bShiftDown);
		CursorPosition = FMath::Max(0, CursorPosition - 1);
		bHandled = true;
	}
	else if (DownKey == EKeys::Right)
	{
		HandleSelectionOnMove(bShiftDown);
		CursorPosition = FMath::Min(Document.GetFullText().Len(), CursorPosition + 1);
		bHandled = true;
	}
	else if (DownKey == EKeys::Up)
	{
		HandleSelectionOnMove(bShiftDown);
		OnUpOrDownPressed(Lines, CursorPos, MyGeometry.Scale, true);
		bHandled = true;
	}
	else if (DownKey == EKeys::Down)
	{
		HandleSelectionOnMove(bShiftDown);
		OnUpOrDownPressed(Lines, CursorPos, MyGeometry.Scale, false);
		bHandled = true;
	}
	else if (DownKey == EKeys::Home)
	{
		CursorPosition = 0;
		bHandled = true;
	}
	else if (DownKey == EKeys::End)
	{
		CursorPosition = Document.GetFullText().Len();
		OnDocumentChanged.Broadcast();
		bHandled = true;
	}
	else if (DownKey == EKeys::Enter)
	{
		DrawSpecialCharacter('\n');
		bHandled = true;
	}
	else if (DownKey == EKeys::Tab)
	{
		DrawSpecialCharacter('\t');
		bHandled = true;
	}
	else if (bControlDown && DownKey == EKeys::A)
	{
		bHandled = false;
	}
	else if (bControlDown && DownKey == EKeys::Z)
	{
		bHandled = false;
	}
	else if (bControlDown && DownKey == EKeys::Y)
	{
		bHandled = false;
	}
	else if (bControlDown && DownKey == EKeys::B)
	{
		ActiveFormat.bIsBold = !ActiveFormat.bIsBold;
		BoldCheckbox->SetIsChecked(ActiveFormat.bIsBold ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		bHandled = true;
	}
	else if (bControlDown && DownKey == EKeys::I)
	{
		ActiveFormat.bIsItalic = !ActiveFormat.bIsItalic;
		ItalicCheckbox->SetIsChecked(ActiveFormat.bIsItalic ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		bHandled = true;
	}
	else if (bControlDown && DownKey == EKeys::U)
	{
		ActiveFormat.bIsUnderline = !ActiveFormat.bIsUnderline;
		UnderlineCheckbox->SetIsChecked(ActiveFormat.bIsUnderline ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		bHandled = true;
	}

	if (bHandled)
	{
		SyncActiveFormat();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

// Requests keyboard focus when the widget is clicked.
FReply SRichTextEditor::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));

	CursorPosition = GetAreaCursorPosition(MouseEvent);
	SelectionAnchor = -1;

	return FReply::Handled();
}

// Extends the active selection by updating CursorPosition to the character under the mouse while the left button is held.
FReply SRichTextEditor::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		CursorPosition = GetAreaCursorPosition(MouseEvent);

		return FReply::Handled();
	}
	return FReply::Unhandled();
}

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
					SAssignNew(TextAreaRef, SRichTextArea).Document(&Document)
														  .CursorPosition(&CursorPosition)
														  .SelectionAnchor(&SelectionAnchor)
				]
		];

	Document.Runs.Add(FRichTextRun(TEXT(""), FCoreStyle::GetDefaultFontStyle("Regular", 12)));
	ActiveFormat.FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", 12);
}

// Sets the bold flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleBold(bool bEnable)
{
	if (bIsSyncing)
	{
		return;
	}
	ActiveFormat.bIsBold = bEnable;
}

// Sets the italic flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleItalic(bool bEnable)
{
	if (bIsSyncing)
	{
		return;
	}
	ActiveFormat.bIsItalic = bEnable;
}

// Sets the underline flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleUnderline(bool bEnable)
{
	if (bIsSyncing)
	{
		return;
	}
	ActiveFormat.bIsUnderline = bEnable;
}

// Sets the strikethrough flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleStrikethrough(bool bEnable)
{
	if (bIsSyncing)
	{
		return;
	}
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
