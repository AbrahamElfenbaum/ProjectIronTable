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

	/** Draws a single text string at the given X and Y offset within the geometry. Used by OnPaint to render each tab-split segment. */
	void DrawTextSegment(FSlateWindowElementList& OutElements, int32 LayerId,
						 const FGeometry& Geometry, const FString& Text,
						 const FSlateFontInfo& FontInfo, float XOffset, float YOffset,
						 const FLinearColor& Color) const;

	/** Draws a horizontal line at the given X/Y offset with the given width; used for underline and strikethrough decoration. */
	void DrawLine(FSlateWindowElementList& ElementList, uint32 InLayer,
				  const FPaintGeometry& PaintGeometry, const FLinearColor& Color,
				  float XOffset, float YOffset, float Width) const;

	/** Returns the pixel X and Y position of the cursor within the document, accounting for tab stops and newlines. TabSpace is the pre-measured width of a single tab gap in layout coordinates. Used by SRichTextEditor for Up/Down navigation. */
	static FVector2f GetCursorPosition(const FRichTextDocument& InDocument, int32 InCursorPosition, float TabSpace, float InScale);

	/** Returns the layout-space pixel width of the given text string using the given font and scale. Scale division is applied internally — result is in unscaled layout coordinates. */
	static float MeasureText(const FString& Text, const FSlateFontInfo& FontInfo, float InScale);
};

