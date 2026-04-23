// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "RichTextRun.h"
#include "RichTextDocument.generated.h"

/** The full content of a single notes channel, stored as a flat ordered list of formatted text runs. */
USTRUCT(BlueprintType)
struct PROJECTIRONTABLE_API FRichTextDocument
{
	GENERATED_BODY()

	/** The ordered list of text runs that make up this document. */
	UPROPERTY()
	TArray<FRichTextRun> Runs;
};