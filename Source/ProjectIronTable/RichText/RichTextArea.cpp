// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "RichTextArea.h"

#include "Fonts/FontMeasure.h"

#include "RichTextDocument.h"
#include "FunctionLibrary.h"

// Stores the document reference for use during painting.
void SRichTextArea::Construct(const FArguments& InArgs)
{
	Document = InArgs._Document;
	CursorPosition = InArgs._CursorPosition;
	SelectionAnchor = InArgs._SelectionAnchor;
}

// Returns the FontInfo of the run that owns the character at CharIndex. Falls back to a default FSlateFontInfo if CharIndex is out of range.
FSlateFontInfo SRichTextArea::FindFontAtIndex(const FRichTextDocument& InDocument, int32 CharIndex)
{
	int32 CharCount = 0;
	for (const FRichTextRun& Run : InDocument.Runs)
	{
		if (CharCount + Run.Text.Len() > CharIndex)
		{
			return Run.FontInfo;
		}
		CharCount += Run.Text.Len();
	}

	return FSlateFontInfo();
}

// Clears and rebuilds VisualLines by walking CachedText character by character, breaking on hard newlines and soft word-wrap boundaries.
void SRichTextArea::RebuildVisualLines(const FSlateFontInfo& InFontInfo, float InScale, float InTabSpace) const
{
	VisualLines.Empty();
	int32 CurrentLineStart = 0;
	float LineWidth = 0.f;
	int32 LastSpaceIndex = -1;

	for (int32 i = 0; i < CachedText.Len(); i++)
	{
		switch (CachedText[i])
		{
		case '\n':
			VisualLines.Add(FVisualLine(CurrentLineStart, i));
			CurrentLineStart = i + 1;
			LineWidth = 0.f;
			LastSpaceIndex = -1;
			break;

		case '\t':
			if (LineWidth + InTabSpace >= CachedWidth)
			{
				VisualLines.Add(FVisualLine(CurrentLineStart, i));
				CurrentLineStart = i;
				LineWidth = InTabSpace;
				LastSpaceIndex = -1;
			}
			else
			{
				LineWidth += InTabSpace;
			}
			break;

		case ' ':
		{
			float CharLength = MeasureText(FString(1, &CachedText[i]), InFontInfo, InScale);
			if (LineWidth + CharLength >= CachedWidth)
			{
				VisualLines.Add(FVisualLine(CurrentLineStart, i));
				CurrentLineStart = i;
				LineWidth = CharLength;
				LastSpaceIndex = -1;
			}
			else
			{
				LastSpaceIndex = i;
				LineWidth += CharLength;
			}
			break;
		}

		default:
		{
			float CharLength = MeasureText(FString(1, &CachedText[i]), InFontInfo, InScale);
			if (LineWidth + CharLength >= CachedWidth)
			{
				if (LastSpaceIndex == -1)
				{
					VisualLines.Add(FVisualLine(CurrentLineStart, i));
					CurrentLineStart = i;
					LineWidth = CharLength;
				}
				else
				{
					VisualLines.Add(FVisualLine(CurrentLineStart, LastSpaceIndex));
					CurrentLineStart = LastSpaceIndex + 1;
					LineWidth = MeasureText(CachedText.Mid(LastSpaceIndex + 1, i - LastSpaceIndex), InFontInfo, InScale);
				}
				LastSpaceIndex = -1;
			}
			else
			{
				LineWidth += CharLength;
			}
			break;
		}
		}
	}

	VisualLines.Add(FVisualLine(CurrentLineStart, CachedText.Len()));
}

// Returns zero width and a height based on visual line count; falls back to hard newline count before the first paint populates VisualLines.
FVector2D SRichTextArea::ComputeDesiredSize(float InLayoutScaleMultiplier) const
{
	if (!Document)
	{
		return FVector2D::ZeroVector;
	}

	if (VisualLines.IsEmpty())
	{
		TArray<FString> Lines = Document->GetLines();
		float LineHeight = UFunctionLibrary::GetDocumentLineHeight(*Document, InLayoutScaleMultiplier);
		return FVector2D(0.f, FMath::Max(1, Lines.Num()) * LineHeight);
	}

	float LineHeight = UFunctionLibrary::GetDocumentLineHeight(*Document, InLayoutScaleMultiplier);
	return FVector2D(0.f, FMath::Max(1, VisualLines.Num()) * LineHeight);
}

// Iterates document runs and draws each as text within the allotted geometry.
int32 SRichTextArea::OnPaint(const FPaintArgs& InArgs, const FGeometry& InAllottedGeometry, const FSlateRect& InMyCullingRect, FSlateWindowElementList& InOutDrawElements,
	int32 InLayerId, const FWidgetStyle& InInWidgetStyle, bool InbParentEnabled) const
{
	if (Document)
	{
		if (!Document->Runs.IsEmpty())
		{
			if (CursorPosition)
			{
				float Scale = InAllottedGeometry.Scale;
				FSlateFontInfo BestFontInfo;
				float LineHeight = UFunctionLibrary::GetDocumentLineHeight(*Document, Scale, &BestFontInfo);
				float TabSpace = MeasureText(TEXT("    "), BestFontInfo, Scale);

				if (CachedWidth < 0 ||
					CachedWidth != InAllottedGeometry.GetLocalSize().X ||
					CachedText != Document->GetFullText())
				{
					CachedWidth = InAllottedGeometry.GetLocalSize().X;
					CachedText = Document->GetFullText();
					RebuildVisualLines(BestFontInfo, Scale, TabSpace);
				}

				if (SelectionAnchor && 
					*SelectionAnchor != -1 &&
					*SelectionAnchor != *CursorPosition)
				{
					FVector2f HighlightStartPos = GetCursorPosition(*Document, FMath::Min(*SelectionAnchor, *CursorPosition),
						TabSpace, Scale);

					FVector2f HighlightEndPos = GetCursorPosition(*Document, FMath::Max(*SelectionAnchor, *CursorPosition),
						TabSpace, Scale);

					DrawHighlight(InOutDrawElements, InLayerId, InAllottedGeometry,
						FLinearColor(0.2f, 0.5f, 1.0f, 0.5f), BestFontInfo, 
						HighlightStartPos, HighlightEndPos, LineHeight, Scale);
				}
				

				float YOffset = 0;
				float XOffset = 0;

				for (const FVisualLine& VisualLine : VisualLines)
				{
					XOffset = 0;
					int32 RunStart = 0;
					int32 RunEnd = 0;

					for (FRichTextRun Run : Document->Runs)
					{
						RunEnd = RunStart + Run.Text.Len();
						if (RunStart >= VisualLine.EndIndex)
						{
							break;
						}

						if (RunEnd <= VisualLine.StartIndex)
						{
							RunStart = RunEnd;
							continue;
						}

						int32 ClipStart = FMath::Max(VisualLine.StartIndex, RunStart) - RunStart;
						int32 ClipEnd = FMath::Min(VisualLine.EndIndex, RunEnd) - RunStart;
						

						FSlateFontInfo FontInfo = Run.FontInfo;

						if (!Run.bIsBold && !Run.bIsItalic)
						{
							FontInfo.TypefaceFontName = FName(TEXT("Regular"));
						}
						else if (Run.bIsBold && !Run.bIsItalic)
						{
							FontInfo.TypefaceFontName = FName(TEXT("Bold"));
						}
						else if (!Run.bIsBold && Run.bIsItalic)
						{
							FontInfo.TypefaceFontName = FName(TEXT("Italic"));
						}
						else
						{
							FontInfo.TypefaceFontName = FName(TEXT("BoldItalic"));
						}

						FString RunClip = Run.Text.Mid(ClipStart, ClipEnd - ClipStart);

						TArray<FString> TabSegments;
						RunClip.ParseIntoArray(TabSegments, TEXT("\t"), false);

						for (int32 k = 0; k < TabSegments.Num(); k++)
						{
							const FString& TabSegment = TabSegments[k];

							DrawTextSegment(InOutDrawElements, InLayerId,
								InAllottedGeometry, TabSegment,
								FontInfo, XOffset,
								YOffset, InInWidgetStyle.GetColorAndOpacityTint());

							float UnderlineStrikethroughWidth = MeasureText(TabSegment.TrimEnd(), FontInfo, Scale);

							if (Run.bIsUnderline)
							{
								DrawLine(InOutDrawElements, InLayerId,
									InAllottedGeometry.ToPaintGeometry(), InInWidgetStyle.GetColorAndOpacityTint(),
									XOffset, YOffset + LineHeight, UnderlineStrikethroughWidth);
							}

							if (Run.bIsStrikethrough)
							{
								DrawLine(InOutDrawElements, InLayerId,
									InAllottedGeometry.ToPaintGeometry(), InInWidgetStyle.GetColorAndOpacityTint(),
									XOffset, YOffset + LineHeight * 0.5f, UnderlineStrikethroughWidth);
							}

							XOffset += MeasureText(TabSegment, FontInfo, InAllottedGeometry.Scale);
							if (k < TabSegments.Num() - 1)
							{
								XOffset += TabSpace;
							}
						}


						RunStart = RunEnd;
					}

					YOffset += LineHeight;
				}

				FVector2f CursorPos = GetCursorPosition(*Document, *CursorPosition, TabSpace, Scale);
				FSlateDrawElement::MakeLines(InOutDrawElements, InLayerId,
					InAllottedGeometry.ToPaintGeometry(),
					TArray<FVector2f>{ CursorPos, FVector2f(CursorPos.X, CursorPos.Y + LineHeight) },
					ESlateDrawEffect::None, InInWidgetStyle.GetColorAndOpacityTint(), false, 1.0f);
			}
		}
	}
	
	return InLayerId;
}

// Emits a single MakeText draw call at the given X/Y offset within the provided geometry.
void SRichTextArea::DrawTextSegment(FSlateWindowElementList& OutElements, int32 LayerId,
									const FGeometry& Geometry, const FString& Text,
									const FSlateFontInfo& FontInfo, float XOffset,
									float YOffset, const FLinearColor& Color) const
{
	FSlateDrawElement::MakeText(OutElements, LayerId, Geometry.ToPaintGeometry(Geometry.GetLocalSize(),
	FSlateLayoutTransform(FVector2D(XOffset, YOffset))), Text, FontInfo, ESlateDrawEffect::None, Color);
}

// Draws a horizontal line at the given X/Y offset with the given width, used for underline and strikethrough decoration.
void SRichTextArea::DrawLine(FSlateWindowElementList& ElementList, uint32 InLayer,
							 const FPaintGeometry& PaintGeometry, const FLinearColor& Color,
							 float XOffset, float YOffset, float Width) const
{
	FSlateDrawElement::MakeLines(ElementList, InLayer, PaintGeometry,
								 TArray<FVector2f>{FVector2f(XOffset, YOffset), FVector2f(XOffset + Width, YOffset)},
								 ESlateDrawEffect::None, Color, false, 1.0);
}

// Draws a highlight rect for each line segment within the selection range, using StartPos for the first line's left edge and EndPos for the last line's right edge.
void SRichTextArea::DrawHighlight(FSlateWindowElementList& ElementList, uint32 InLayer,
								  const FGeometry& Geometry, const FLinearColor& Color,
								  const FSlateFontInfo& InFontInfo, FVector2f StartPos, 
								  FVector2f EndPos, float LineHeight, float InScale) const
{

	int32 StartLine = StartPos.Y / LineHeight;
	int32 EndLine = EndPos.Y / LineHeight;

	for (int32 i = StartLine; i <= EndLine; i++)
	{
		float TopY = LineHeight * i;
		float LeftX = (i == StartLine ? StartPos.X : 0);
		float RightX = (i == EndLine ? EndPos.X : MeasureText(CachedText.Mid(VisualLines[i].StartIndex, VisualLines[i].EndIndex - VisualLines[i].StartIndex), 
															  InFontInfo, InScale));

		FSlateDrawElement::MakeBox(ElementList, InLayer, 
								   Geometry.ToPaintGeometry(FVector2D(RightX - LeftX, LineHeight), FSlateLayoutTransform(FVector2D(LeftX, TopY))),
								   FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, Color);
	}
}

// Returns the pixel X and Y position of the cursor within the document, based on font measurement and line splitting.
FVector2f SRichTextArea::GetCursorPosition(const FRichTextDocument& InDocument, int32 InCursorPosition, float TabSpace, float InScale) const
{
	float LineHeight = UFunctionLibrary::GetDocumentLineHeight(InDocument, InScale);
	float CursorX = 0;
	float CursorY = 0;

	for (const FVisualLine& VisualLine : VisualLines)
	{
		if (InCursorPosition >= VisualLine.StartIndex && 
			InCursorPosition <= VisualLine.EndIndex)
		{
			TArray<FString> LineSegments;
			CachedText.Mid(VisualLine.StartIndex, VisualLine.EndIndex - VisualLine.StartIndex).ParseIntoArray(LineSegments, TEXT("\t"), false);
			float SegmentOffset = 0;
			int32 SegCharCount = 0;
			for (const FString& LineSegment : LineSegments)
			{
				if (SegCharCount + LineSegment.Len() >= InCursorPosition - VisualLine.StartIndex)
				{
					CursorX = SegmentOffset + MeasureText(LineSegment.Left(InCursorPosition - VisualLine.StartIndex - SegCharCount),
						FindFontAtIndex(InDocument, VisualLine.StartIndex + SegCharCount), InScale) + 1;
					break;
				}
				SegmentOffset += MeasureText(LineSegment, FindFontAtIndex(InDocument, VisualLine.StartIndex + SegCharCount), InScale) + TabSpace;
				SegCharCount += LineSegment.Len() + 1;
			}
			break;
		}
		CursorY += LineHeight;
	}

	return FVector2f(CursorX, CursorY);
}

// Returns the layout-space pixel width of the given string using the font measure service, with DPI scale divided out.
float SRichTextArea::MeasureText(const FString& Text, const FSlateFontInfo& FontInfo, float InScale)
{
	return FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
		->Measure(Text, FontInfo, InScale).X / InScale;
}

// Converts a local-space mouse position to the nearest document character index by finding the target line via Y, then walking characters by X.
int32 SRichTextArea::HitTest(FVector2f LocalMousePos, const FRichTextDocument& InDocument, float InScale) const
{
	float LineHeight = UFunctionLibrary::GetDocumentLineHeight(InDocument, InScale);
	int32 TargetLine = FMath::Clamp((int32)(LocalMousePos.Y / LineHeight), 0, VisualLines.Num() - 1);
	int32 CharInLine = 0;
	float LeftEdge = 0.f;

	for (int32 i = VisualLines[TargetLine].StartIndex; i < VisualLines[TargetLine].EndIndex; i++)
	{
		float RightEdge = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
			->Measure(CachedText.Mid(VisualLines[TargetLine].StartIndex, i - VisualLines[TargetLine].StartIndex + 1), 
					  FindFontAtIndex(InDocument, i), InScale).X / InScale;

		float Midpoint = (RightEdge + LeftEdge) / 2.f;

		if (Midpoint > LocalMousePos.X)
		{
			break;
		}
		CharInLine = i - VisualLines[TargetLine].StartIndex + 1;
		LeftEdge = RightEdge;
	}

	return VisualLines[TargetLine].StartIndex + CharInLine;
}
