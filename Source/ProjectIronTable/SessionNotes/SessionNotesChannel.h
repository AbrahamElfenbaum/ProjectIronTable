// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseChannel.h"
#include "SessionNotesChannel.generated.h"

class UEditableRichText;

/** A single notes channel widget. Holds a rich text editor and a unique ChannelID used as the save key in USessionSave. */
UCLASS()
class PROJECTIRONTABLE_API USessionNotesChannel : public UBaseChannel
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** Editable rich text field where the player writes and formats session notes. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableRichText> Notes;
#pragma endregion

public:

#pragma region State
	/** Unique identifier for this channel, used as the key in USessionSave's notes maps. */
	FGuid ChannelID;
#pragma endregion

private:

#pragma region Private Methods
	/** Scrolls to the end of the scroll box when the notes document changes. */
	void OnNotesChanged();
#pragma endregion

protected:

	/** Generates a unique ChannelID for this channel instance and binds the document-changed delegate. */
	virtual void NativeConstruct() override;

public:

#pragma region Public Methods
	/** Scrolls the channel's scroll box to the bottom. */
	void ScrollToEnd();

	/** Returns the Notes rich text widget so the panel can call GetDocument and SetDocument on it. */
	UEditableRichText* GetNotes();
#pragma endregion
};
