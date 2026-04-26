// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "Widgets/SLeafWidget.h"

struct FRichTextDocument;

/** Slate leaf widget that renders a rich text document as formatted runs. Owned by SRichTextEditor and placed in the text area slot. */
class SRichTextArea : public SLeafWidget
{
private:

	/** The document to render. Provided at construction; not owned by this widget. */
	const FRichTextDocument* Document = nullptr;

	/** Current cursor position as a character index into the document. Pointer into SRichTextEditor's CursorPosition so it stays in sync. */
	const int32* CursorPosition = nullptr;

public:

	SLATE_BEGIN_ARGS(SRichTextArea) {}
		SLATE_ARGUMENT(const FRichTextDocument*, Document)
		SLATE_ARGUMENT(const int32*, CursorPosition)
	SLATE_END_ARGS()

	/** Stores the document reference passed in via SLATE_ARGUMENT. */
	void Construct(const FArguments& InArgs);

	/** Returns the desired size of the text area. */
	FVector2D ComputeDesiredSize(float InLayoutScaleMultiplier) const override;

	/** Draws each document run as text within the allotted geometry. */
	int32 OnPaint(const FPaintArgs& InArgs, const FGeometry& InAllottedGeometry, const FSlateRect& InMyCullingRect, FSlateWindowElementList& InOutDrawElements,
		int32 InLayerId, const FWidgetStyle& InInWidgetStyle, bool InbParentEnabled) const override;

	/** Returns the pixel X and Y position of the cursor within the document, based on font measurement and line splitting. Used by SRichTextEditor for Up/Down navigation. */
	static FVector2f GetCursorPosition(const FRichTextDocument& InDocument, int32 InCursorPosition, float InScale);
};

