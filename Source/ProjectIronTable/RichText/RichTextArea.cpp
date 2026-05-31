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
void SRichTextArea::RebuildVisualLines(float InScale, float InTabSpace) const
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
			float CharLength = MeasureText(FString(1, &CachedText[i]), FindFontAtIndex(*Document, i), InScale);
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
			float CharLength = MeasureText(FString(1, &CachedText[i]), FindFontAtIndex(*Document, i), InScale);
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
					LineWidth = MeasureText(CachedText.Mid(LastSpaceIndex + 1, i - LastSpaceIndex), FindFontAtIndex(*Document, i), InScale);
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

// Returns the max character height across all runs that overlap the given visual line. Falls back to the default font height if no runs overlap.
float SRichTextArea::GetLineHeightForVisualLine(int32 VisualLineIndex, float InScale) const
{
	int32 StartIndex = VisualLines[VisualLineIndex].StartIndex;
	int32 EndIndex = VisualLines[VisualLineIndex].EndIndex;

	int32 RunStart = 0;
	int32 RunEnd = 0;

	float MaxLineHeight = -1.f;
	for (FRichTextRun Run : Document->Runs)
	{
		RunEnd = RunStart + Run.Text.Len();
		if (RunStart >= EndIndex)
		{
			break;
		}

		if (RunEnd <= StartIndex)
		{
			RunStart = RunEnd;
			continue;
		}

		float NewLineHeight = FSlateApplication::Get().GetRenderer()
			->GetFontMeasureService()
			->GetMaxCharacterHeight(Run.FontInfo, InScale);

		MaxLineHeight = FMath::Max(MaxLineHeight, NewLineHeight);

		RunStart = RunEnd;
	}

	if (MaxLineHeight == -1.f)
	{
		return FSlateApplication::Get().GetRenderer()
			->GetFontMeasureService()
			->GetMaxCharacterHeight(FCoreStyle::GetDefaultFontStyle("Regular", 12), InScale);
	}
	return MaxLineHeight;
}

// Returns zero width and a height based on visual line count; falls back to hard newline count before the first paint populates VisualLines.
FVector2D SRichTextArea::ComputeDesiredSize(float InLayoutScaleMultiplier) const
{
	if (!Document)
	{
		return FVector2D::ZeroVector;
	}

	float TotalHeight = 0.f;

	if (VisualLines.IsEmpty())
	{
		TArray<FString> Lines = Document->GetLines();
		TotalHeight = UFunctionLibrary::GetDocumentLineHeight(*Document, InLayoutScaleMultiplier);
		return FVector2D(0.f, FMath::Max(1, Lines.Num()) * TotalHeight);
	}

	for (int32 i = 0; i < VisualLines.Num(); i++)
	{
		TotalHeight += GetLineHeightForVisualLine(i, InLayoutScaleMultiplier);
	}

	return FVector2D(0.f, TotalHeight);
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
				UFunctionLibrary::GetDocumentLineHeight(*Document, Scale, &BestFontInfo);
				float TabSpace = MeasureText(TEXT("    "), BestFontInfo, Scale);

				if (CachedWidth < 0 ||
					CachedWidth != InAllottedGeometry.GetLocalSize().X ||
					CachedText != Document->GetFullText())
				{
					CachedWidth = InAllottedGeometry.GetLocalSize().X;
					CachedText = Document->GetFullText();
					RebuildVisualLines(Scale, TabSpace);
				}

				if (SelectionAnchor && 
					*SelectionAnchor != -1 &&
					*SelectionAnchor != *CursorPosition)
				{
					FVector2f HighlightStartPos = GetCursorPosition(*Document, FMath::Min(*SelectionAnchor, *CursorPosition), Scale);
					FVector2f HighlightEndPos = GetCursorPosition(*Document, FMath::Max(*SelectionAnchor, *CursorPosition), Scale);

					DrawHighlight(InOutDrawElements, InLayerId, InAllottedGeometry,
						FLinearColor(0.2f, 0.5f, 1.0f, 0.5f), 
						HighlightStartPos, HighlightEndPos, Scale);
				}
				

				float YOffset = 0;
				float XOffset = 0;

				for (int32 LineIndex = 0; LineIndex < VisualLines.Num(); LineIndex++)
				{
					float CurrentLineHeight = GetLineHeightForVisualLine(LineIndex, Scale);
					
					XOffset = 0;
					int32 RunStart = 0;
					int32 RunEnd = 0;

					for (FRichTextRun Run : Document->Runs)
					{
						FSlateFontInfo RunFontInfo = Run.FontInfo;
						
						RunEnd = RunStart + Run.Text.Len();
						if (RunStart >= VisualLines[LineIndex].EndIndex)
						{
							break;
						}

						if (RunEnd <= VisualLines[LineIndex].StartIndex)
						{
							RunStart = RunEnd;
							continue;
						}

						int32 ClipStart = FMath::Max(VisualLines[LineIndex].StartIndex, RunStart) - RunStart;
						int32 ClipEnd = FMath::Min(VisualLines[LineIndex].EndIndex, RunEnd) - RunStart;

						if (!Run.bIsBold && !Run.bIsItalic)
						{
							RunFontInfo.TypefaceFontName = FName(TEXT("Regular"));
						}
						else if (Run.bIsBold && !Run.bIsItalic)
						{
							RunFontInfo.TypefaceFontName = FName(TEXT("Bold"));
						}
						else if (!Run.bIsBold && Run.bIsItalic)
						{
							RunFontInfo.TypefaceFontName = FName(TEXT("Italic"));
						}
						else
						{
							RunFontInfo.TypefaceFontName = FName(TEXT("BoldItalic"));
						}

						float RunTabSpace = MeasureText(TEXT("    "), RunFontInfo, Scale);
						FString RunClip = Run.Text.Mid(ClipStart, ClipEnd - ClipStart);

						TArray<FString> TabSegments;
						RunClip.ParseIntoArray(TabSegments, TEXT("\t"), false);

						for (int32 k = 0; k < TabSegments.Num(); k++)
						{
							const FString& TabSegment = TabSegments[k];

							DrawTextSegment(InOutDrawElements, InLayerId,
								InAllottedGeometry, TabSegment,
								RunFontInfo, XOffset,
								YOffset, InInWidgetStyle.GetColorAndOpacityTint());

							float UnderlineStrikethroughWidth = MeasureText(TabSegment.TrimEnd(), RunFontInfo, Scale);

							if (Run.bIsUnderline)
							{
								DrawLine(InOutDrawElements, InLayerId,
									InAllottedGeometry.ToPaintGeometry(), InInWidgetStyle.GetColorAndOpacityTint(),
									XOffset, YOffset + CurrentLineHeight, UnderlineStrikethroughWidth);
							}

							if (Run.bIsStrikethrough)
							{
								DrawLine(InOutDrawElements, InLayerId,
									InAllottedGeometry.ToPaintGeometry(), InInWidgetStyle.GetColorAndOpacityTint(),
									XOffset, YOffset + CurrentLineHeight * 0.5f, UnderlineStrikethroughWidth);
							}

							XOffset += MeasureText(TabSegment, RunFontInfo, InAllottedGeometry.Scale);
							if (k < TabSegments.Num() - 1)
							{
								XOffset += RunTabSpace;
							}
						}

						RunStart = RunEnd;
					}

					YOffset += CurrentLineHeight;
				}

				int32 LineIndex = 0;
				for (int32 i = 0; i < VisualLines.Num(); i++)
				{
					if (*CursorPosition >= VisualLines[i].StartIndex &&
						*CursorPosition <= VisualLines[i].EndIndex)
					{
						LineIndex = i;
						break;
					}
				}

				FVector2f CursorPos = GetCursorPosition(*Document, *CursorPosition, Scale);
				
				
				FSlateDrawElement::MakeLines(InOutDrawElements, InLayerId,
					InAllottedGeometry.ToPaintGeometry(),
					TArray<FVector2f>{ CursorPos, FVector2f(CursorPos.X, CursorPos.Y + GetLineHeightForVisualLine(LineIndex, Scale)) },
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
								  FVector2f StartPos, FVector2f EndPos, float InScale) const
{
	float AccumY = 0.f;
	int32 StartLineIndex = VisualLines.Num() - 1;
	for (int32 i = 0; i < VisualLines.Num(); i++)
	{
		float H = GetLineHeightForVisualLine(i, InScale);
		if (StartPos.Y < AccumY + H)
		{
			StartLineIndex = i;
			break;
		}
		AccumY += H;
	}

	AccumY = 0.f;
	int32 EndLineIndex = VisualLines.Num() - 1;
	for (int32 i = 0; i < VisualLines.Num(); i++)
	{
		float H = GetLineHeightForVisualLine(i, InScale);
		if (EndPos.Y < AccumY + H)
		{
			EndLineIndex = i;
			break;
		}
		AccumY += H;
	}

	float TopY = 0.f;
	for (int32 j = 0; j < StartLineIndex; j++)
	{
		TopY += GetLineHeightForVisualLine(j, InScale);
	}

	for (int32 i = StartLineIndex; i <= EndLineIndex; i++)
	{
		float CurrentLineHeight = GetLineHeightForVisualLine(i, InScale);
		
		float LeftX = (i == StartLineIndex ? StartPos.X : 0);
		float RightX = 0.f;

		if (i == EndLineIndex)
		{
			RightX = EndPos.X;
		}
		else
		{
			int32 RunStart = 0;
			int32 RunEnd = 0;
			for (FRichTextRun Run : Document->Runs)
			{
				RunEnd = RunStart + Run.Text.Len();
				if (RunStart >= VisualLines[i].EndIndex)
				{
					break;
				}
				if (RunEnd <= VisualLines[i].StartIndex)
				{
					RunStart = RunEnd;
					continue;
				}
				int32 ClipStart = FMath::Max(VisualLines[i].StartIndex, RunStart) - RunStart;
				int32 ClipEnd = FMath::Min(VisualLines[i].EndIndex, RunEnd) - RunStart;
				RightX += MeasureText(Run.Text.Mid(ClipStart, ClipEnd - ClipStart), Run.FontInfo, InScale);
				RunStart = RunEnd;
			}
		}

		FSlateDrawElement::MakeBox(ElementList, InLayer, 
								   Geometry.ToPaintGeometry(FVector2D(RightX - LeftX, CurrentLineHeight), FSlateLayoutTransform(FVector2D(LeftX, TopY))),
								   FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, Color);
		TopY += CurrentLineHeight;
	}
}

// Returns the pixel X and Y position of the cursor within the document, walking runs per-line for correct per-font measurement.
FVector2f SRichTextArea::GetCursorPosition(const FRichTextDocument& InDocument, int32 InCursorPosition, float InScale) const
{
	float CursorX = 0;
	float CursorY = 0;

	for (int32 i = 0; i < VisualLines.Num(); i++)
	{
		if (InCursorPosition >= VisualLines[i].StartIndex &&
			InCursorPosition <= VisualLines[i].EndIndex)
		{
			int32 RunStart = 0;
			int32 RunEnd = 0;
			for (const FRichTextRun& Run : InDocument.Runs)
			{
				RunEnd = RunStart + Run.Text.Len();
				if (RunStart >= VisualLines[i].EndIndex) break;
				if (RunEnd <= VisualLines[i].StartIndex) { RunStart = RunEnd; continue; }

				int32 DocClipStart = FMath::Max(VisualLines[i].StartIndex, RunStart);
				int32 DocClipEnd = FMath::Min(VisualLines[i].EndIndex, RunEnd);

				FSlateFontInfo FontInfo = Run.FontInfo;
				float RunTabSpace = MeasureText(TEXT("    "), FontInfo, InScale);

				TArray<FString> TabSegs;
				CachedText.Mid(DocClipStart, DocClipEnd - DocClipStart).ParseIntoArray(TabSegs, TEXT("\t"), false);

				int32 SegDocPos = DocClipStart;
				bool bCursorInRun = false;
				for (int32 k = 0; k < TabSegs.Num(); k++)
				{
					const FString& Seg = TabSegs[k];
					int32 SegDocEnd = SegDocPos + Seg.Len();

					if (InCursorPosition >= SegDocPos && InCursorPosition <= SegDocEnd)
					{
						CursorX += MeasureText(Seg.Left(InCursorPosition - SegDocPos), FontInfo, InScale) + 1;
						bCursorInRun = true;
						break;
					}

					CursorX += MeasureText(Seg, FontInfo, InScale);
					if (k < TabSegs.Num() - 1)
					{
						CursorX += RunTabSpace;
						SegDocPos += Seg.Len() + 1;
					}
					else
					{
						SegDocPos += Seg.Len();
					}
				}

				if (bCursorInRun) break;
				RunStart = RunEnd;
			}
			break;
		}
		CursorY += GetLineHeightForVisualLine(i, InScale);
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

	float AccumY = 0.f;
	int32 TargetLine = VisualLines.Num() - 1;
	for (int32 i = 0; i < VisualLines.Num(); i++)
	{
		float H = GetLineHeightForVisualLine(i, InScale);
		if (LocalMousePos.Y < AccumY + H)
		{
			TargetLine = i;
			break;
		}
		AccumY += H;
	}

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
