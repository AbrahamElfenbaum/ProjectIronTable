// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameTypeTab.generated.h"

class UButton;
class UTextBlock;

/** Fired when the tab button is clicked, passing the game type name this tab represents. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameTypeSelected, const FString&, GameType);

/** A tab button representing a single game type in the Campaign Manager. Displays the type name and fires OnGameTypeSelected when clicked. */
UCLASS()
class PROJECTIRONTABLE_API UGameTypeTab : public UUserWidget
{
	GENERATED_BODY()

public:

#pragma region Events
	/** Fired when the button is clicked, passing the game type name. */
	UPROPERTY(BlueprintAssignable)
	FOnGameTypeSelected OnGameTypeSelected;
#pragma endregion

#pragma region Public Methods
	/** Sets the displayed game type label. */
	void SetLabel(const FString& Label);

	/** Sets the selected and unselected background colors used by SetSelected. */
	void SetTabColors(const FLinearColor& InSelectedTabColor, const FLinearColor& InUnselectedTabColor);

	/** Returns the displayed game type label text. */
	FString GetLabel() const;

	/** Enables or disables the button — used to grey it out when no campaigns exist for this game type. */
	void SetInteractable(bool bInteractable);

	/** Sets the button's selected state, visually indicating it is the active game type. */
	void SetSelected(bool bSelected);
#pragma endregion

protected:
	/** Binds the tab button click delegate. */
	virtual void NativeConstruct() override;

private:

#pragma region Widget References
	/** Button the player clicks to select this game type. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GameTypeTab;

	/** Label displaying the game type name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GameTypeLabel;
#pragma endregion

#pragma region State
	/** Background color applied to the button when it is the active game type. */
	FLinearColor SelectedTabColor;

	/** Background color applied to the button when it is not the active game type. */
	FLinearColor UnselectedTabColor;
#pragma endregion

#pragma region Event Handlers
	/** Broadcasts OnGameTypeSelected with the current label text as the game type name. */
	UFUNCTION()
	void OnGameTypeTabClicked();
#pragma endregion
};
