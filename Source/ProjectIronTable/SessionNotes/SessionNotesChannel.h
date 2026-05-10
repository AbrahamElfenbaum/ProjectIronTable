// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseChannel.h"
#include "SessionNotesChannel.generated.h"

class UEditableRichText;
class UCheckBox;

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

	/** Toolbar checkbox for toggling bold formatting on the active selection or newly typed text. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> BoldCheckBox;

	/** Toolbar checkbox for toggling italic formatting on the active selection or newly typed text. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> ItalicCheckBox;

	/** Toolbar checkbox for toggling underline formatting on the active selection or newly typed text. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> UnderlineCheckBox;

	/** Toolbar checkbox for toggling strikethrough formatting on the active selection or newly typed text. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> StrikethroughCheckBox;
#pragma endregion

public:

#pragma region State
	/** Unique identifier for this channel, used as the key in USessionSave's notes maps. */
	FGuid ChannelID;

	/** Guards against feedback loops when OnFormatChanged programmatically sets checkbox states — prevents the resulting OnCheckStateChanged callbacks from calling Toggle* on the editor. */
	bool bIsSyncing = false;
#pragma endregion

private:

#pragma region Private Methods
	/** Scrolls to the end of the scroll box when the notes document changes. */
	void OnNotesChanged();
#pragma endregion

#pragma region Event Handlers

	/** Forwards the bold checkbox state to the rich text editor when the user toggles it. */
	UFUNCTION()
	void OnBoldCheckStateChanged(bool bIsChecked);

	/** Forwards the italic checkbox state to the rich text editor when the user toggles it. */
	UFUNCTION()
	void OnItalicCheckStateChanged(bool bIsChecked);

	/** Forwards the underline checkbox state to the rich text editor when the user toggles it. */
	UFUNCTION()
	void OnUnderlineCheckStateChanged(bool bIsChecked);

	/** Forwards the strikethrough checkbox state to the rich text editor when the user toggles it. */
	UFUNCTION()
	void OnStrikethroughCheckStateChanged(bool bIsChecked);

	/** Receives format state broadcasts from the rich text editor and updates the toolbar checkboxes, guarded by bIsSyncing to prevent feedback loops. */
	UFUNCTION()
	void OnFormatChanged(bool bBold, bool bItalic, bool bUnderline, bool bStrikethrough);
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
