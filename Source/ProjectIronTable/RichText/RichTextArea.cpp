// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "RichTextArea.h"
#include "RichTextDocument.h"
#include "Fonts/FontMeasure.h"

// Stores the document reference for use during painting.
void SRichTextArea::Construct(const FArguments& InArgs)
{
	Document = InArgs._Document;
	CursorPosition = InArgs._CursorPosition;
}

// Returns zero until document content is measured.
FVector2D SRichTextArea::ComputeDesiredSize(float InLayoutScaleMultiplier) const
{
	return FVector2D();
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
				float TabSpace = MeasureText(TEXT("    "), Document->Runs[0].FontInfo, InAllottedGeometry.Scale);

				float LineHeight = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
					->GetMaxCharacterHeight(Document->Runs[0].FontInfo, InAllottedGeometry.Scale);

				TArray<FString> Lines;
				Document->GetFullText().ParseIntoArray(Lines, TEXT("\n"), false);
				float YOffset = 0;

				//Draw Text
				for (FString Line : Lines)
				{
					TArray<FString> LineSegments;
					Line.ParseIntoArray(LineSegments, TEXT("\t"), false);
					float XOffset = 0;
					for (FString LineSegment : LineSegments)
					{
						DrawTextSegment(InOutDrawElements, InLayerId,
							InAllottedGeometry, LineSegment,
							Document->Runs[0].FontInfo, XOffset,
							YOffset, InInWidgetStyle.GetColorAndOpacityTint());

						XOffset += MeasureText(LineSegment, Document->Runs[0].FontInfo, InAllottedGeometry.Scale);
						XOffset += TabSpace;
					}
					YOffset += LineHeight;
				}

				//Draw Cursor
				FVector2f CursorPos = GetCursorPosition(*Document, *CursorPosition, TabSpace, InAllottedGeometry.Scale);
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

// Returns the pixel X and Y position of the cursor within the document, based on font measurement and line splitting.
FVector2f SRichTextArea::GetCursorPosition(const FRichTextDocument& InDocument, int32 InCursorPosition, float TabSpace, float InScale)
{
	TArray<FString> Lines;
	InDocument.GetFullText().ParseIntoArray(Lines, TEXT("\n"), false);

	float LineHeight = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
		->GetMaxCharacterHeight(InDocument.Runs[0].FontInfo, InScale);

	float CursorX = 0;
	float CursorY = 0;

	int32 CharCount = 0;

	for (FString Line : Lines)
	{
		if (CharCount + Line.Len() >= InCursorPosition)
		{
			TArray<FString> LineSegments;
			Line.ParseIntoArray(LineSegments, TEXT("\t"), false);
			float SegmentOffset = 0;
			int32 SegCharCount = 0;
			for (FString LineSegment : LineSegments)
			{
				if (SegCharCount + LineSegment.Len() >= InCursorPosition - CharCount)
				{
					CursorX = SegmentOffset + MeasureText(LineSegment.Left(InCursorPosition - CharCount - SegCharCount), InDocument.Runs[0].FontInfo, InScale) + 1;
					break;
				}
				SegCharCount += LineSegment.Len() + 1;
				SegmentOffset += MeasureText(LineSegment, InDocument.Runs[0].FontInfo, InScale) + TabSpace;
			}
			break;
		}
		CursorY += LineHeight;
		CharCount += Line.Len();
		CharCount++;
	}

	return FVector2f(CursorX, CursorY);
}

// Returns the layout-space pixel width of the given string using the font measure service, with DPI scale divided out.
float SRichTextArea::MeasureText(const FString& Text, const FSlateFontInfo& FontInfo, float InScale)
{
	return FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
		->Measure(Text, FontInfo, InScale).X / InScale;
}
