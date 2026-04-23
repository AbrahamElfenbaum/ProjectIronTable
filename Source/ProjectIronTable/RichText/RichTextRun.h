// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "RichTextRun.generated.h"

/** A contiguous range of characters that share the same formatting. The document model is a flat list of these runs. */
USTRUCT(BlueprintType)
struct PROJECTIRONTABLE_API FRichTextRun
{
	GENERATED_BODY()

	/** Whether this run is rendered in bold. */
	UPROPERTY()
	bool bIsBold = false;

	/** Whether this run is rendered in italic. */
	UPROPERTY()
	bool bIsItalic = false;

	/** Whether this run is rendered with an underline. */
	UPROPERTY()
	bool bIsUnderline = false;

	/** Whether this run is rendered with a strikethrough line. */
	UPROPERTY()
	bool bIsStrikethrough = false;

	/** The text content of this run. */
	UPROPERTY()
	FString Text = TEXT("");

	/** Font typeface and size used to render this run. */
	UPROPERTY()
	FSlateFontInfo FontInfo;

	/** Default constructor — required by UE's reflection system. FontInfo is left empty; must be set before rendering. */
	FRichTextRun() {}

	/** Constructs a run with the given text and font. All formatting flags default to false. */
	FRichTextRun(const FString& InText, const FSlateFontInfo& InFont)
		: bIsBold(false), bIsItalic(false), bIsUnderline(false), bIsStrikethrough(false),
		  Text(InText), FontInfo(InFont)
	{}
};