// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContextMenuButton.generated.h"

DECLARE_DELEGATE(FOnContextMenuButtonClicked);

/** Data for a single context menu entry: display label and the callback to invoke when clicked. */
USTRUCT(BlueprintType)
struct FContextMenuOption
{
    GENERATED_BODY()

    /** Text displayed on the menu button. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ButtonName;

    FOnContextMenuButtonClicked OnClicked;
};

class UButton;
class UTextBlock;

/** A single button entry in a context menu, configured with a label and a callback delegate. */
UCLASS()
class PROJECTIRONTABLE_API UContextMenuButton : public UUserWidget
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** The clickable button widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MenuButton;

	/** Displays the option label text. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ButtonLabel;
#pragma endregion

#pragma region State
	/** The option data this button represents, including its label and callback. */
	FContextMenuOption Option;
#pragma endregion

public:

#pragma region Public Methods
	/** Assigns the option to this button, updates the label, and binds the click handler. */
	void SetOption(const FContextMenuOption& InOption);
#pragma endregion

private:

#pragma region Event Handlers
	/** Executes the option's OnClicked delegate when the button is pressed. */
	UFUNCTION()
	void OnMenuButtonClicked();
#pragma endregion
};
