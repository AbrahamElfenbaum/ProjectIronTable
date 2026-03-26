// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatEntry.h"

void UChatEntry::NativeConstruct()
{
	Super::NativeConstruct();

	TextBlock->SetText(FText::FromString(Message));
}
