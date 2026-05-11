// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "Widgets/SLeafWidget.h"

/** Represents a single visual line in the rendered document as a range of character indices into the full document text. Not pixel positions — indices only. */
struct FVisualLine
{
	/** Character index of the first character on this visual line, into the full document text. */
	int32 StartIndex;

	/** Character index one past the last character on this visual line, into the full document text. */
	int32 EndIndex;

	/** Initializes both indices to zero. */
	FVisualLine()
	{
		StartIndex = 0;
		EndIndex = 0;
	}

	/** Initializes with the given start and end character indices. */
	FVisualLine(const int32 InStartIndex, const int32 InEndIndex)
	{
		StartIndex = InStartIndex;
		EndIndex = InEndIndex;
	}
};

struct FRichTextDocument;

/** Slate leaf widget that renders a rich text document as formatted runs. Owned by SRichTextEditor and placed in the text area slot. */
class SRichTextArea : public SLeafWidget
{
private:

	/** The document to render. Provided at construction; not owned by this widget. */
	const FRichTextDocument* Document = nullptr;

	/** Current cursor position as a character index into the document. Pointer into SRichTextEditor's CursorPosition so it stays in sync. */
	const int32* CursorPosition = nullptr;
	
	/** Active selection anchor as a character index into the document. Pointer into SRichTextEditor's SelectionAnchor so it stays in sync. -1 when no selection is active. */
	const int32* SelectionAnchor = nullptr;

	/** Last allotted width used to build the line layout cache. -1 until first paint. Recompute triggered when this differs from the current allotted width. */
	mutable float CachedWidth = -1.f;

	/** Snapshot of the document's full text at the last layout recompute. Recompute triggered when this differs from the current document text. */
	mutable FString CachedText;

	/** Pre-computed character index ranges for each visual line, rebuilt whenever the document text or allotted width changes. StartIndex and EndIndex are character indices into the full document text — not pixel positions. Covers both hard newline breaks and soft word-wrap breaks. */
	mutable TArray<FVisualLine> VisualLines;

	/** Returns the FontInfo of the run that owns the character at CharIndex. Falls back to a default FSlateFontInfo if CharIndex is out of range. */
	static FSlateFontInfo FindFontAtIndex(const FRichTextDocument& InDocument, int32 CharIndex);

	/** Walks CachedText character by character and populates VisualLines with one FVisualLine per rendered line, handling hard newlines, tab overflow, space-boundary word wrap, and whole-word wrap when no space is available. */
	void RebuildVisualLines(const FSlateFontInfo& InFontInfo, float InScale, float InTabSpace) const;

public:

	SLATE_BEGIN_ARGS(SRichTextArea) {}
		SLATE_ARGUMENT(const FRichTextDocument*, Document)
		SLATE_ARGUMENT(const int32*, CursorPosition)
		SLATE_ARGUMENT(const int32*, SelectionAnchor)
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

	/** Draws a highlight rect behind each line segment that falls within the selection, from StartPos on the first line to EndPos on the last. */
	void DrawHighlight(FSlateWindowElementList& ElementList, uint32 InLayer,
					   const FGeometry& Geometry, const FLinearColor& Color,
					   const TArray<FString>& InLines, const FSlateFontInfo& InFontInfo,
					   FVector2f StartPos, FVector2f EndPos, float LineHeight, float InScale) const;

	/** Returns the pixel X and Y position of the cursor within the document, accounting for tab stops and newlines. TabSpace is the pre-measured width of a single tab gap in layout coordinates. Used by SRichTextEditor for Up/Down navigation. */
	static FVector2f GetCursorPosition(const FRichTextDocument& InDocument, int32 InCursorPosition, float TabSpace, float InScale);

	/** Returns the layout-space pixel width of the given text string using the given font and scale. Scale division is applied internally — result is in unscaled layout coordinates. */
	static float MeasureText(const FString& Text, const FSlateFontInfo& FontInfo, float InScale);

	/** Converts a local-space mouse position to the nearest document character index by finding the target line via Y, then walking characters by X. */
	static int32 HitTest(FVector2f LocalMousePos, const FRichTextDocument& InDocument, float InScale);
};

