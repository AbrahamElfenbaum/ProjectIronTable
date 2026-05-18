// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "RichTextEditor.h"
#include "Engine/Font.h"

#include "Fonts/FontMeasure.h"

#include "RichTextArea.h"
#include "RichTextRun.h"
#include "FunctionLibrary.h"

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
void SRichTextEditor::OnUpOrDownPressed(FVector2f CursorPos, float Scale, float TabSpace, bool bUp)
{
	FSlateFontInfo FontInfo;
	float LineHeight = UFunctionLibrary::GetDocumentLineHeight(Document, Scale, &FontInfo);

	int32 CurrentLine = CursorPos.Y / LineHeight;
	int32 TargetLine = CurrentLine + (bUp ? -1 : 1);

	if (PreferredX < 0)
	{
		PreferredX = CursorPos.X;
	}

	if (!((bUp && TargetLine < 0) || (!bUp && TargetLine >= TextAreaRef->GetVisualLines().Num())))
	{
		int32 CharInLine = 0;
		float XOffset = 0.f;

		int32 EndIndex = TextAreaRef->GetVisualLines()[TargetLine].EndIndex - TextAreaRef->GetVisualLines()[TargetLine].StartIndex;

		for (int32 i = 0; i < EndIndex; i++)
		{
			TCHAR CurrChar = TextAreaRef->GetCachedText()[TextAreaRef->GetVisualLines()[TargetLine].StartIndex + i];
			float CharWidth = (CurrChar == '\t' ? TabSpace :
												 FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
												 ->Measure(FString(1, &CurrChar), FontInfo, Scale).X / Scale);

			float Midpoint = XOffset + CharWidth / 2.f;

			if (Midpoint > PreferredX)
			{
				break;
			}
			CharInLine = i + 1;
			XOffset += CharWidth;
		}

		CursorPosition = TextAreaRef->GetVisualLines()[TargetLine].StartIndex + CharInLine;
	}
}

// Copies the format flags of the run under CursorPosition into ActiveFormat so newly typed text inherits the correct formatting.
void SRichTextEditor::SyncActiveFormat()
{
	int32 RunStart = 0;
	int32 RunIndex = FindRunAtIndex(CursorPosition, RunStart);
	ActiveFormat = Document.Runs[RunIndex];
	OnFormatStateChanged.Broadcast(ActiveFormat.bIsBold, ActiveFormat.bIsItalic, ActiveFormat.bIsUnderline, ActiveFormat.bIsStrikethrough);
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
	return TextAreaRef->HitTest(LocalMousePos, Document, TextAreaRef->GetTickSpaceGeometry().Scale);
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
	if (bShiftDown)
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

// Deletes SelectionMax - SelectionMin characters at SelectionMin, then moves the cursor to SelectionMin and clears the selection.
void SRichTextEditor::RangeDelete()
{
	int32 SelectionMin = GetSelectionMin();
	int32 SelectionMax = GetSelectionMax();

	for (int i = 0; i < SelectionMax - SelectionMin; i++)
	{
		OnBackspaceOrDeletePressed(SelectionMin);
	}
	CursorPosition = SelectionMin;
	SelectionAnchor = -1;
}

// Splits runs at SelectionMin and SelectionMax, applies ApplyFormat to all runs within the selection, then prunes and merges.
void SRichTextEditor::FormatToSelection(TFunction<void(FRichTextRun&)> ApplyFormat)
{
	if (SelectionAnchor != -1)
	{
		int32 Min = GetSelectionMin();
		int32 MinRunStart = 0;
		int32 MinRunIndex = FindRunAtIndex(Min, MinRunStart);
		int32 SelectionStartIndex = SplitRunAt(MinRunIndex, Min - MinRunStart);

		int32 Max = GetSelectionMax();
		int32 MaxRunStart = 0;
		int32 MaxRunIndex = FindRunAtIndex(Max, MaxRunStart);
		int32 SelectionEndIndex = SplitRunAt(MaxRunIndex, Max - MaxRunStart);

		for (int32 i = SelectionStartIndex; i <= SelectionEndIndex; i++)
		{
			ApplyFormat(Document.Runs[i]);
		}

		PruneRuns();
	}
}

// Splits the run at RunIndex into Left and Right pieces at LocalOffset, replacing the original. Returns the index of the Right piece.
int32 SRichTextEditor::SplitRunAt(int32 RunIndex, int32 LocalOffset)
{
	FRichTextRun FoundRun = Document.Runs[RunIndex];

	FRichTextRun LeftRun = FoundRun;
	LeftRun.Text = FoundRun.Text.Left(LocalOffset);

	FRichTextRun RightRun = FoundRun;
	RightRun.Text = FoundRun.Text.Mid(LocalOffset);

	Document.Runs.RemoveAt(RunIndex);

	Document.Runs.Insert(LeftRun, RunIndex);
	Document.Runs.Insert(RightRun, RunIndex + 1);

	return RunIndex + 1;
}

// Toggles Flag, applies the format change to the active selection via Apply, and broadcasts the new format state. Returns FReply::Handled().
FReply SRichTextEditor::ApplyFormatShortcut(bool& Flag, TFunction<void(FRichTextRun&)> Apply)
{
	Flag = !Flag;
	FormatToSelection(Apply);
	OnFormatStateChanged.Broadcast(ActiveFormat.bIsBold, ActiveFormat.bIsItalic, ActiveFormat.bIsUnderline, ActiveFormat.bIsStrikethrough);
	return FReply::Handled();
}

// Inserts the typed character into the correct run at CursorPosition and advances the cursor.
FReply SRichTextEditor::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent)
{
	TCHAR Character = InCharacterEvent.GetCharacter();

	if (Character < 32)
	{
		return FReply::Unhandled();
	}

	if (SelectionAnchor != -1)
	{
		RangeDelete();
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
	bool bDocumentModified = false;
	bool bHandled = false;
	bool bVerticalMove = false;
	FKey DownKey = InKeyEvent.GetKey();
	bool bControlDown = InKeyEvent.IsControlDown();
	bool bShiftDown = InKeyEvent.IsLeftShiftDown() || InKeyEvent.IsRightShiftDown();
	FSlateFontInfo FontInfo;
	float LineHeight = UFunctionLibrary::GetDocumentLineHeight(Document, MyGeometry.Scale, &FontInfo);

	float TabSpace = SRichTextArea::MeasureText(TEXT("    "), FontInfo, MyGeometry.Scale);
	FVector2f CursorPos = TextAreaRef->GetCursorPosition(Document, CursorPosition, TabSpace, MyGeometry.Scale);

	if (DownKey == EKeys::BackSpace)
	{
		if (SelectionAnchor != -1)
		{
			RangeDelete();
			SyncActiveFormat();
			PreferredX = -1.f;
			OnDocumentChanged.Broadcast();
			return FReply::Handled();
		}

		if (CursorPosition != 0)
		{
			bDocumentModified = true;
			OnBackspaceOrDeletePressed(CursorPosition - 1);
			CursorPosition = FMath::Max(0, CursorPosition - 1);
			bHandled = true;
		}
	}
	else if (DownKey == EKeys::Delete)
	{
		if (SelectionAnchor != -1)
		{
			RangeDelete();
			SyncActiveFormat();
			PreferredX = -1.f;
			OnDocumentChanged.Broadcast();
			return FReply::Handled();
		}

		if (CursorPosition != Document.GetFullText().Len())
		{
			bDocumentModified = true;
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
		OnUpOrDownPressed(CursorPos, MyGeometry.Scale, TabSpace, true);
		bVerticalMove = true;
		bHandled = true;
	}
	else if (DownKey == EKeys::Down)
	{
		HandleSelectionOnMove(bShiftDown);
		OnUpOrDownPressed(CursorPos, MyGeometry.Scale, TabSpace, false);
		bVerticalMove = true;
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
		bHandled = true;
	}
	else if (DownKey == EKeys::Enter)
	{
		//bDocumentModified = true;
		if (SelectionAnchor != -1)
		{
			RangeDelete();
		}
		DrawSpecialCharacter('\n');
		OnDocumentChanged.Broadcast();
		return FReply::Handled();
		//bHandled = true;
	}
	else if (DownKey == EKeys::Tab)
	{
		//bDocumentModified = true;
		if (SelectionAnchor != -1)
		{
			RangeDelete();
		}
		DrawSpecialCharacter('\t');
		OnDocumentChanged.Broadcast();
		return FReply::Handled();
		//bHandled = true;
	}
	else if (bControlDown && DownKey == EKeys::A)
	{
		SelectionAnchor = 0;
		CursorPosition = Document.GetFullText().Len();
		bHandled = true;
	}
	else if (bControlDown && DownKey == EKeys::B)
	{
		return ApplyFormatShortcut(ActiveFormat.bIsBold,
								   [this](FRichTextRun& Run) { Run.bIsBold = ActiveFormat.bIsBold; });
	}
	else if (bControlDown && DownKey == EKeys::C)
	{
		if (SelectionAnchor != -1)
		{
			CopiedRuns = GetSelectedRange();
			bHandled = true;
		}
	}
	else if (bControlDown && DownKey == EKeys::I)
	{
		return ApplyFormatShortcut(ActiveFormat.bIsItalic,
								   [this](FRichTextRun& Run) { Run.bIsItalic = ActiveFormat.bIsItalic; });
	}
	else if (bControlDown && DownKey == EKeys::U)
	{
		return ApplyFormatShortcut(ActiveFormat.bIsUnderline,
								   [this](FRichTextRun& Run) { Run.bIsUnderline = ActiveFormat.bIsUnderline; });
	}
	else if (bControlDown && DownKey == EKeys::V)
	{
		if (!CopiedRuns.IsEmpty())
		{
			bDocumentModified = true;
			if (SelectionAnchor != -1)
			{
				RangeDelete();
			}

			int32 RunStart = 0;
			int32 TotalCharacterCount = 0;
			int32 RunIndex = FindRunAtIndex(CursorPosition, RunStart);
			int32 SelectionIndex = SplitRunAt(RunIndex, CursorPosition - RunStart);

			for (FRichTextRun& Run : CopiedRuns)
			{
				Document.Runs.Insert(Run, SelectionIndex);
				TotalCharacterCount += Run.Text.Len();
				SelectionIndex++;
			}

			CursorPosition += TotalCharacterCount;

			PruneRuns();
			bHandled = true;
		}
	}
	else if (bControlDown && DownKey == EKeys::X)
	{
		if (SelectionAnchor != -1)
		{
			bDocumentModified = true;
			CopiedRuns = GetSelectedRange();
			RangeDelete();
			bHandled = true;
		}
	}
	else if (bControlDown && DownKey == EKeys::Y)
	{
		bHandled = false;
	}
	else if (bControlDown && DownKey == EKeys::Z)
	{
		bHandled = false;
	}

	if (!bVerticalMove)
	{
		PreferredX = -1.f;
	}

	if (bHandled)
	{
		if (bDocumentModified)
		{
			OnDocumentChanged.Broadcast();
		}
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
	PreferredX = -1.f;

	return FReply::Handled();
}

// Extends the active selection by updating CursorPosition to the character under the mouse while the left button is held.
FReply SRichTextEditor::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		int32 NewCursorPos = GetAreaCursorPosition(MouseEvent);
		if (SelectionAnchor == -1 && CursorPosition != NewCursorPos)
		{
			SelectionAnchor = CursorPosition;
		}

		CursorPosition = NewCursorPos;

		return FReply::Handled();
	}
	return FReply::Unhandled();
}

// Builds the editor layout with a text area child and initializes the document and active format font.
void SRichTextEditor::Construct(const FArguments& InArgs)
{
	ChildSlot
		[
			SAssignNew(TextAreaRef, SRichTextArea).Document(&Document)
				.CursorPosition(&CursorPosition)
				.SelectionAnchor(&SelectionAnchor)
		];

	Document.Runs.Add(FRichTextRun(TEXT(""), FCoreStyle::GetDefaultFontStyle("Regular", 12)));
	ActiveFormat.FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", 12);
}

// Sets the bold flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleBold(bool bEnable)
{
	ActiveFormat.bIsBold = bEnable;
	FormatToSelection([bEnable](FRichTextRun& Run) { Run.bIsBold = bEnable; });
}

// Sets the italic flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleItalic(bool bEnable)
{
	ActiveFormat.bIsItalic = bEnable;
	FormatToSelection([bEnable](FRichTextRun& Run) { Run.bIsItalic = bEnable; });
}

// Sets the underline flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleUnderline(bool bEnable)
{
	ActiveFormat.bIsUnderline = bEnable;
	FormatToSelection([bEnable](FRichTextRun& Run) { Run.bIsUnderline = bEnable; });
}

// Sets the strikethrough flag on ActiveFormat, applying to newly typed text and any current selection.
void SRichTextEditor::ToggleStrikethrough(bool bEnable)
{
	ActiveFormat.bIsStrikethrough = bEnable;
	FormatToSelection([bEnable](FRichTextRun& Run) { Run.bIsStrikethrough = bEnable; });
}

// Builds a new FSlateFontInfo from InFont preserving the current size and typeface variant, then applies to ActiveFormat and any current selection.
void SRichTextEditor::SetFontStyle(UFont* InFont)
{
	FSlateFontInfo NewFontInfo(InFont, ActiveFormat.FontInfo.Size, ActiveFormat.FontInfo.TypefaceFontName);
	ActiveFormat.FontInfo = NewFontInfo;
	FormatToSelection([NewFontInfo](FRichTextRun& Run) { Run.FontInfo = NewFontInfo; });
}

// Updates ActiveFormat font size and applies to any current selection.
void SRichTextEditor::SetFontSize(int32 InFontSize)
{
	ActiveFormat.FontInfo.Size = InFontSize;
	FormatToSelection([InFontSize](FRichTextRun& Run) { Run.FontInfo.Size = InFontSize; });
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
