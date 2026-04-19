// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionNotesPanel.generated.h"

class UScrollBox;
class UMultiLineEditableText;
class USessionNotesChannel;
class USessionNotesTab;
class USessionNotesChannelListEntry;
class UContextMenu;

/** Widget panel that provides a scrollable, editable multi-line text area for session notes. */
UCLASS()
class PROJECTIRONTABLE_API USessionNotesPanel : public UUserWidget
{
	GENERATED_BODY()

public:

#pragma region Config
	/** Widget class used when creating new session notes channel widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<USessionNotesChannel> ChannelClass;

	/** Widget class used when creating new session notes tab widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<USessionNotesTab> TabClass;

	/** Widget class used when creating entries in the closed channel list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<USessionNotesChannelListEntry> SessionNotesListEntryClass;

	/** Widget class used for the context menu when right-clicking a channel tab. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UContextMenu> ContextMenuClass;
#pragma endregion


private:

#pragma region Widget References
	/** Scroll box wrapping the notes text area; scrolled to the end when new content is added. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> NotesScroll;

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

};
