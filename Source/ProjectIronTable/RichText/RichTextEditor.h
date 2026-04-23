// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "RichTextDocument.h"

class SMultiLineEditableText;
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

	/** The multiline text input area where the user types. */
	TSharedPtr<SMultiLineEditableText> TextArea;

	/** Toolbar checkbox for toggling bold formatting. */
	TSharedPtr<SCheckBox> BoldCheckbox;

	/** Toolbar checkbox for toggling italic formatting. */
	TSharedPtr<SCheckBox> ItalicCheckbox;

	/** Toolbar checkbox for toggling underline formatting. */
	TSharedPtr<SCheckBox> UnderlineCheckbox;

	/** Toolbar checkbox for toggling strikethrough formatting. */
	TSharedPtr<SCheckBox> StrikethroughCheckbox;

	/** Builds a single toolbar checkbox wired to the given format callback and labeled with the given string. */
	TSharedRef<SWidget> MakeFormatCheckbox(TSharedPtr<SCheckBox>& Checkbox, TFunction<void(bool)> Callback, const TCHAR* Label);
};
