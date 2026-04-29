// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseChannelPanel.h"
#include "SessionNotesPanel.generated.h"

/** Widget panel that manages multiple tabbed notes channels, each containing an editable rich text document. */
UCLASS()
class PROJECTIRONTABLE_API USessionNotesPanel : public UBaseChannelPanel
{
	GENERATED_BODY()

private:

#pragma region Private Methods
	/** Returns a display label for a new notes tab — "Notes" for the first, "Notes 2", "Notes 3", etc. for subsequent tabs. */
	virtual FString CreateTabLabel(const TArray<FString>& Participants) const override;

	/** No-op — save is handled directly in CreateChannel. */
	virtual void SaveCreatedTab() override;

	/** Updates the saved tab label in USessionSave when the user renames a notes tab. */
	virtual void OnChannelRenamed(UBaseChannelTab* Tab, const FString& NewName, const FString& ParticipantsKey) override;

	/** No-op — the channel widget retains its in-memory document on switch. */
	virtual void OnChannelSwitched(UBaseChannel* Channel) override;
#pragma endregion

public:

#pragma region Public Methods
	/** Creates a new USessionNotesChannel, assigns it a GUID, saves initial state to USessionSave, and returns it. */
	virtual UBaseChannel* CreateChannel(const TArray<FString>& Participants) override;
#pragma endregion
};
