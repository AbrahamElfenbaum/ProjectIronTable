// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContextMenuButton.h"
#include "ContextMenu.generated.h"

class USizeBox;
class UVerticalBox;

/** A dynamic context menu widget that spawns a vertical list of buttons from a set of options. */
UCLASS()
class PROJECTIRONTABLE_API UContextMenu : public UUserWidget
{
	GENERATED_BODY()

public:

#pragma region Config
	/** The widget class to spawn for each context menu entry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UContextMenuButton> ContextMenuButtonClass;
#pragma endregion

protected:

	FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:

#pragma region Widget References
	/** Outermost size container; auto-fits to menu content. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> ContextBox;

	/** Vertical layout container for spawned button entries. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> MenuOptionsBox;
#pragma endregion

#pragma region State
	/** Cached references to all currently spawned menu buttons. */
	UPROPERTY()
	TArray<TObjectPtr<UContextMenuButton>> MenuOptions;
#pragma endregion

public:

#pragma region Public Methods
	/** Clears existing buttons and spawns a new button for each provided option. */
	void SetMenuOptions(const TArray<FContextMenuOption>& Options);

	/** */
	void CloseMenu();
#pragma endregion
};
