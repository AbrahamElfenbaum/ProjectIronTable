// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatEntry.h"

#include "Components/TextBlock.h"

// Populates the text block with the message string set before this widget was added to the viewport.
void UChatEntry::NativeConstruct()
{
	Super::NativeConstruct();

	TextBlock->SetText(FText::FromString(Message));
}
