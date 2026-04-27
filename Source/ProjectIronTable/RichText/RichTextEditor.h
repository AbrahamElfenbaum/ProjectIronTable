// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "RichTextDocument.h"

class SCheckBox;

class SRichTextEditor : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRichTextEditor) {}
	SLATE_END_ARGS()

	/** Initializes the widget layout. Called by Slate when the widget is first created. */
	void Construct(const FArguments& InArgs);

	/** Sets whether newly typed text and any selected text will be rendered in bold. */
	void ToggleBold(bool bEnable);

	/** Sets whether newly typed text and any selected text will be rendered in italic. */
	void ToggleItalic(bool bEnable);

	/** Sets whether newly typed text and any selected text will be rendered with an underline. */
	void ToggleUnderline(bool bEnable);

	/** Sets whether newly typed text and any selected text will be rendered with a strikethrough. */
	void ToggleStrikethrough(bool bEnable);

	/** Returns the current document for serialization. */
	FRichTextDocument GetDocument() const;

	/** Replaces the current document with the given one, used when loading saved content. */
	void SetDocument(const FRichTextDocument& InDocument);

	/** Returns true so the widget can receive keyboard focus. */
	bool SupportsKeyboardFocus() const override;

protected:

	/** Handles character input events. */
	FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent) override;

	/** Handles key down events for cursor movement, deletion, and format shortcuts. */
	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	/** Requests keyboard focus when the widget is clicked. */
	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:

	/** Start of the active text selection as a character index. -1 when nothing is selected. */
	int32 SelectionStart = -1;

	/** End of the active text selection as a character index. -1 when nothing is selected. */
	int32 SelectionEnd = -1;

	/** Current cursor position as a character index into the document. */
	int32 CursorPosition = 0;

	/** The document being edited, stored as an ordered list of formatted runs. */
	FRichTextDocument Document;

	/** Formatting state applied to newly typed text. Updated when the cursor moves or a format toggle is pressed. Text field is unused — this is a format carrier only. */
	FRichTextRun ActiveFormat;

	/** Toolbar checkbox for toggling bold formatting. */
	TSharedPtr<SCheckBox> BoldCheckbox;

	/** Toolbar checkbox for toggling italic formatting. */
	TSharedPtr<SCheckBox> ItalicCheckbox;

	/** Toolbar checkbox for toggling underline formatting. */
	TSharedPtr<SCheckBox> UnderlineCheckbox;

	/** Toolbar checkbox for toggling strikethrough formatting. */
	TSharedPtr<SCheckBox> StrikethroughCheckbox;

	/***/
	bool bIsSyncing;

	/** Finds the run in Document.Runs that spans CharIndex and sets OutRunStart to the character index where that run begins. Returns the index of the run in Document.Runs. */
	int32 FindRunAtIndex(int32 CharIndex, int32& OutRunStart) const;

	/** Builds a single toolbar checkbox wired to the given format callback and labeled with the given string. */
	TSharedRef<SWidget> MakeFormatCheckbox(TSharedPtr<SCheckBox>& Checkbox, TFunction<void(bool)> Callback, const TCHAR* Label);

	/** Returns true if both runs share the same bold, italic, underline, strikethrough flags and font info. */
	bool FormatsMatch(const FRichTextRun& A, const FRichTextRun& B) const;

	/** Inserts a non-printable character (e.g. newline, tab) at CursorPosition using the same run-walk as OnKeyChar, then advances the cursor. */
	FReply DrawSpecialCharacter(TCHAR SpecialCharacter);

	/** Removes the character at the given document index, resolving the correct run using FindRunAtIndex and redirecting to the next run if the index falls on a run boundary. */
	void OnBackspaceOrDeletePressed(int32 CursorPos);

	/** Moves the cursor up or down one line, landing on the character closest to the current X pixel position. */
	FReply OnUpOrDownPressed(const TArray<FString>& Lines, FVector2f CursorPos, float Scale, bool bUp);

	/** Updates ActiveFormat to match the format flags of the run currently under the cursor. Called after any operation that moves CursorPosition. */
	void SyncActiveFormat();

	/** Removes all empty runs from the document; re-adds a blank default run using ActiveFormat if none remain. */
	void PruneRuns();
};
