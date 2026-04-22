// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseChannelPanel.h"
#include "SessionNotesPanel.generated.h"

class UMultiLineEditableText;

/** Widget panel that provides a scrollable, editable multi-line text area for session notes. */
UCLASS()
class PROJECTIRONTABLE_API USessionNotesPanel : public UBaseChannelPanel
{
	GENERATED_BODY()
private:

#pragma region Widget References
	/** Editable multi-line text field where the player writes session notes. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMultiLineEditableText> NotesText;
#pragma endregion

protected:
	/** Binds the text changed delegate to keep the scroll position at the bottom as content grows. */
	virtual void NativeConstruct() override;

private:

#pragma region Event Handlers
	/** Scrolls the notes panel to the end whenever the text content changes. */
	UFUNCTION()
	void OnNotesTextChanged(const FText& Text);
#pragma endregion

public:

#pragma region Public Methods
	/** Creates a new USessionNotesChannel, assigns it a GUID, and returns it. Calls Super to handle tab wiring. */
	UBaseChannel* CreateChannel(const TArray<FString>& Participants) override;

	/** Returns a display label for a new notes tab (e.g. "Notes", "Notes 2"). */
	FString CreateTabLabel(const TArray<FString>& Participants) const override;

	/** Persists the newly created tab's GUID and label to USessionSave. */
	void SaveCreatedTab() override;

	/** Updates the saved tab label in USessionSave when the user renames a notes tab. */
	void OnChannelRenamed(UBaseChannelTab* Tab, const FString& NewName, const FString& ParticipantsKey) override;

	/** Loads saved content for the switched-to channel and displays it in the notes field. */
	void OnChannelSwitched(UBaseChannel* Channel) override;
#pragma endregion
};
