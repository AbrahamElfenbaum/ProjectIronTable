// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionNotesChannel.h"

#include "Components/ScrollBox.h"

// Scrolls the channel's scroll box to the bottom; logs a warning if ScrollBox is not bound.
void USessionNotesChannel::ScrollToEnd()
{
	if (!ScrollBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionNotesChannel::ScrollToEnd — ScrollBox is null"));
		return;
	}
	ScrollBox->ScrollToEnd();
}
