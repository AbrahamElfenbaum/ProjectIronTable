// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionNotesPanel.generated.h"

class UScrollBox;
class UMultiLineEditableText;

/** Widget panel that provides a scrollable, editable multi-line text area for session notes. */
UCLASS()
class PROJECTIRONTABLE_API USessionNotesPanel : public UUserWidget
{
	GENERATED_BODY()

private:
	/** Scroll box wrapping the notes text area; scrolled to the end when new content is added. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> NotesScroll;

	/** Editable multi-line text field where the player writes session notes. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMultiLineEditableText> NotesText;

protected:
	/** Binds the text changed delegate to keep the scroll position at the bottom as content grows. */
	virtual void NativeConstruct() override;

private:

	/** Scrolls the notes panel to the end whenever the text content changes. */
	UFUNCTION()
	void OnNotesTextChanged(const FText& Text);
	
};
