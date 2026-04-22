// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseChannel.h"
#include "SessionNotesChannel.generated.h"

/** A single notes channel widget. Extends UBaseChannel with scroll-to-end behavior for use in USessionNotesPanel. */
UCLASS()
class PROJECTIRONTABLE_API USessionNotesChannel : public UBaseChannel
{
	GENERATED_BODY()

public:

	/** Scrolls the channel's scroll box to the bottom. */
	void ScrollToEnd();
};
