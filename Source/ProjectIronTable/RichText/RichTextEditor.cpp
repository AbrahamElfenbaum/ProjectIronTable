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
	int32 RunStart = 0;

	if (Character < 32)
	{
		return FReply::Unhandled();
	}

	for (FRichTextRun& Run : Document.Runs)
	{
		int32 RunEnd = RunStart + Run.Text.Len();
		if (CursorPosition >= RunStart && CursorPosition <= RunEnd)
		{
			Run.Text.InsertAt(CursorPosition - RunStart, Character);
			CursorPosition++;
			break;
		}
		RunStart = RunEnd;
	}

	return FReply::Handled();
}

// Handles cursor movement, deletion, and format shortcut keys.
FReply SRichTextEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	FKey DownKey = InKeyEvent.GetKey();
	bool bControlDown = InKeyEvent.IsControlDown();

	TArray<FString> Lines;
	Document.Runs[0].Text.ParseIntoArray(Lines, TEXT("\n"), false);
	FVector2f CursorPos = SRichTextArea::GetCursorPosition(Document, CursorPosition, MyGeometry.Scale);

	if (DownKey == EKeys::BackSpace)
	{
		if (CursorPosition != 0)
		{
			Document.Runs[0].Text.RemoveAt(CursorPosition - 1);
			CursorPosition = FMath::Max(0, CursorPosition - 1);
			return FReply::Handled();
		}

	}
	else if (DownKey == EKeys::Delete)
	{
		if (CursorPosition != Document.Runs[0].Text.Len())
		{
			Document.Runs[0].Text.RemoveAt(CursorPosition);
			return FReply::Handled();
		}
	}
	else if (DownKey == EKeys::Left)
	{
		CursorPosition = FMath::Max(0, CursorPosition - 1);
		return FReply::Handled();
	}
	else if (DownKey == EKeys::Right)
	{
		CursorPosition = FMath::Min(Document.Runs[0].Text.Len(), CursorPosition + 1);
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
		return FReply::Handled();
	}
	else if (DownKey == EKeys::End)
	{
		CursorPosition = Document.Runs[0].Text.Len();
		return FReply::Handled();
	}
	else if (DownKey == EKeys::Enter)
	{
		int32 RunStart = 0;
		for (FRichTextRun& Run : Document.Runs)
		{
			int32 RunEnd = RunStart + Run.Text.Len();
			if (CursorPosition >= RunStart && CursorPosition <= RunEnd)
			{
				Run.Text.InsertAt(CursorPosition - RunStart, '\n');
				CursorPosition++;
				break;
			}
			RunStart = RunEnd;
		}

		return FReply::Handled();
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
		return FReply::Unhandled();
	}
	else if (bControlDown && DownKey == EKeys::I)
	{
		return FReply::Unhandled();
	}
	else if (bControlDown && DownKey == EKeys::U)
	{
		return FReply::Unhandled();
	}

	return FReply::Handled();
}

// Requests keyboard focus when the widget is clicked.
FReply SRichTextEditor::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	return FReply::Handled();
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

	return FReply::Handled();
}
