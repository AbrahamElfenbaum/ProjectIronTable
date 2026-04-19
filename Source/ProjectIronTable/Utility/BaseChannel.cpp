// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "BaseChannel.h"

#include "Components/ScrollBox.h"

// Adjusts the scroll offset by ScrollMultiplier in the requested direction, clamped to valid range.
void UBaseChannel::Scroll(bool bUp)
{
	if (!ScrollBox) return;

	int32 ScrollDirection = bUp ? 1 : -1;
	ScrollBox->SetScrollOffset(
		FMath::Clamp(
			ScrollBox->GetScrollOffset() + (ScrollMultiplier * ScrollDirection),
			0.0f,
			ScrollBox->GetScrollOffsetOfEnd()));
}