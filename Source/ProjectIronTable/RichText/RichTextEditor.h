// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "RichTextDocument.h"

class SCheckBox;
class SRichTextArea;

DECLARE_MULTICAST_DELEGATE(FOnDocumentChanged)

class SRichTextEditor : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRichTextEditor) {}
	SLATE_END_ARGS()

#pragma region Fields

private:

	/** Start of the active text selection as a character index. -1 when nothing is selected. */
	int32 SelectionAnchor = -1;

	/** Current cursor position as a character index into the document. */
	int32 CursorPosition = 0;

	/** Target X pixel position held across consecutive Up/Down presses to prevent column drift. Set to the cursor's X on the first vertical move, reset to -1 on any non-vertical key or mouse click. */
	float PreferredX = -1.f;

	/** The document being edited, stored as an ordered list of formatted runs. */
	FRichTextDocument Document;

	/** Formatting state applied to newly typed text. Updated when the cursor moves or a format toggle is pressed. Text field is unused — this is a format carrier only. */
	FRichTextRun ActiveFormat;

	/** Runs copied by the last Ctrl+C or Ctrl+X operation, used as the source for Ctrl+V paste. */
	TArray<FRichTextRun> CopiedRuns;

	/** Cached pointer to the text area child widget, used to resolve mouse positions relative to the text area rather than the full editor geometry. */
	TSharedPtr<SRichTextArea> TextAreaRef;

	/** Toolbar checkbox for toggling bold formatting. */
	TSharedPtr<SCheckBox> BoldCheckbox;

	/** Toolbar checkbox for toggling italic formatting. */
	TSharedPtr<SCheckBox> ItalicCheckbox;

	/** Toolbar checkbox for toggling underline formatting. */
	TSharedPtr<SCheckBox> UnderlineCheckbox;

	/** Toolbar checkbox for toggling strikethrough formatting. */
	TSharedPtr<SCheckBox> StrikethroughCheckbox;

	/** Guards against feedback loops when programmatically calling SetIsChecked on format checkboxes — SCheckBox fires OnCheckStateChanged with the previous state on programmatic updates, which would overwrite ActiveFormat. Set to true around all SetIsChecked calls in SyncActiveFormat; toggle callbacks return early while true. */
	bool bIsSyncing;

public:

	/** Fired when the document content changes due to user input or format commands. */
	FOnDocumentChanged OnDocumentChanged;

#pragma endregion

#pragma region Functions

private:

	/** Finds the run in Document.Runs that spans CharIndex and sets OutRunStart to the character index where that run begins. Returns the index of the run in Document.Runs. */
	int32 FindRunAtIndex(int32 CharIndex, int32& OutRunStart) const;

	/** Builds a single toolbar checkbox wired to the given format callback and labeled with the given string. */
	TSharedRef<SWidget> MakeFormatCheckbox(TSharedPtr<SCheckBox>& Checkbox, TFunction<void(bool)> Callback, const TCHAR* Label);

	/** Returns true if both runs share the same bold, italic, underline, strikethrough flags and font info. */
	bool FormatsMatch(const FRichTextRun& A, const FRichTextRun& B) const;

	/** Inserts a non-printable character (e.g. newline, tab) at CursorPosition using the same run-walk as OnKeyChar, then advances the cursor. */
	void DrawSpecialCharacter(TCHAR SpecialCharacter);

	/** Removes the character at the given document index, resolving the correct run using FindRunAtIndex and redirecting to the next run if the index falls on a run boundary. */
	void OnBackspaceOrDeletePressed(int32 CursorPos);

	/** Moves the cursor up or down one line, landing on the character closest to the current X pixel position. */
	void OnUpOrDownPressed(const TArray<FString>& Lines, FVector2f CursorPos, float Scale, bool bUp);

	/** Updates ActiveFormat to match the format flags of the run currently under the cursor. Called after any operation that moves CursorPosition. */
	void SyncActiveFormat();

	/** Removes all empty runs, merges adjacent runs with identical format, and re-adds a blank default run if none remain. */
	void PruneRuns();

	/** Returns the lower bound of the active selection as a character index. */
	int32 GetSelectionMin() const { return FMath::Min(SelectionAnchor, CursorPosition); }

	/** Returns the upper bound of the active selection as a character index. */
	int32 GetSelectionMax() const { return FMath::Max(SelectionAnchor, CursorPosition); }

	/** Converts a mouse event's screen-space position to a character index using TextAreaRef's geometry, correcting for the toolbar height offset. */
	int32 GetAreaCursorPosition(const FPointerEvent& MouseEvent);

	/** Returns a copy of the document runs that fall within the active selection, with each run's text clipped to the selection boundaries. */
	TArray<FRichTextRun> GetSelectedRange();

	/** Sets SelectionAnchor to the current CursorPosition when Shift is first held and a move begins, or clears it to -1 when Shift is not held. Must be called before CursorPosition is updated. */
	void HandleSelectionOnMove(bool bShiftDown);

	/** Removes all characters in [SelectionMin, SelectionMax) from the document, moves the cursor to SelectionMin, and clears the selection. */
	void RangeDelete();

	/** Applies a format change to all runs within the active selection, splitting runs at SelectionMin and SelectionMax as needed so partial runs are handled correctly. */
	void FormatToSelection(TFunction<void(FRichTextRun&)> ApplyFormat);

	/** Splits the run at RunIndex into Left and Right pieces at LocalOffset, replacing the original in Document.Runs. Returns the index of the Right piece. */
	int32 SplitRunAt(int32 RunIndex, int32 LocalOffset);

	/** Toggles Flag, applies the format change to the active selection via Apply, and syncs the Checkbox state. Returns FReply::Handled(). */
	FReply ApplyFormatShortcut(bool& Flag, TSharedPtr<SCheckBox>& Checkbox, TFunction<void(FRichTextRun&)> Apply);

protected:

	/** Handles character input events. */
	FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent) override;

	/** Handles key down events for cursor movement, deletion, and format shortcuts. */
	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	/** Requests keyboard focus when the widget is clicked. */
	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** Extends the active selection by updating CursorPosition to the character under the mouse while the left button is held. */
	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

public:

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

#pragma endregion

};
