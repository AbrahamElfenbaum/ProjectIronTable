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
				uint16 LineHeight = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
					->GetMaxCharacterHeight(Document->Runs[0].FontInfo, InAllottedGeometry.Scale);

				TArray<FString> Lines;
				Document->Runs[0].Text.ParseIntoArray(Lines, TEXT("\n"), false);
				uint16 YOffset = 0;
				for (FString Line : Lines)
				{
					FSlateDrawElement::MakeText(InOutDrawElements, InLayerId, InAllottedGeometry.ToPaintGeometry(InAllottedGeometry.GetLocalSize(),
						FSlateLayoutTransform(FVector2D(0, YOffset))), Line,
						Document->Runs[0].FontInfo, ESlateDrawEffect::None, InInWidgetStyle.GetColorAndOpacityTint());
					YOffset += LineHeight;
				}

				FVector2f CursorPos = GetCursorPosition(*Document, *CursorPosition, InAllottedGeometry.Scale);
				FSlateDrawElement::MakeLines(InOutDrawElements, InLayerId,
					InAllottedGeometry.ToPaintGeometry(),
					TArray<FVector2f>{ CursorPos, FVector2f(CursorPos.X, CursorPos.Y + LineHeight) },
					ESlateDrawEffect::None, InInWidgetStyle.GetColorAndOpacityTint(), false, 1.0f);

			}
		}
	}
	
	return InLayerId;
}

// Returns the pixel X and Y position of the cursor within the document, based on font measurement and line splitting.
FVector2f SRichTextArea::GetCursorPosition(const FRichTextDocument& InDocument, int32 InCursorPosition, float InScale)
{
	TArray<FString> Lines;
	InDocument.Runs[0].Text.ParseIntoArray(Lines, TEXT("\n"), false);

	uint16 LineHeight = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
		->GetMaxCharacterHeight(InDocument.Runs[0].FontInfo, InScale);

	float CursorX = 0;
	float CursorY = 0;

	int32 CharCount = 0;

	for (FString Line : Lines)
	{
		if (CharCount + Line.Len() >= InCursorPosition)
		{
			CursorX = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()
					  ->Measure(Line.Left(InCursorPosition - CharCount), 
								InDocument.Runs[0].FontInfo, InScale).X / InScale + 1;

			break;
		}
		CursorY += LineHeight;
		CharCount += Line.Len();
		CharCount++;
	}

	return FVector2f(CursorX, CursorY);
}
