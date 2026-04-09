// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameTypeButton.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameTypeSelected, const FString&, GameType);

/** A tab button representing a single game type in the Campaign Manager. Displays the type name and fires OnGameTypeSelected when clicked. */
UCLASS()
class PROJECTIRONTABLE_API UGameTypeButton : public UUserWidget
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** Button the player clicks to select this game type. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GameTypeTab;

	/** Label displaying the game type name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GameTypeLabel;
#pragma endregion


protected:

	/** Binds the tab button click delegate. */
	virtual void NativeConstruct() override;

public:

	/** Fired when the button is clicked, passing the game type name. */
	UPROPERTY(BlueprintAssignable)
	FOnGameTypeSelected OnGameTypeSelected;

	/** Sets the displayed game type label. */
	void SetLabel(const FString& Label);

	/** Enables or disables the button — used to grey it out when no campaigns exist for this game type. */
	void SetInteractable(bool bInteractable);

private:

	/** Broadcasts OnGameTypeSelected with the current label text as the game type name. */
	UFUNCTION()
	void OnGameTypeButtonClicked();

};
