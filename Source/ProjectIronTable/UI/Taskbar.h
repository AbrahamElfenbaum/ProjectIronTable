// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Taskbar.generated.h"

class UTaskbarButton;
class UButton;

/**
 * Taskbar widget displayed at the bottom of the screen. Holds toggle buttons for all registered HUD widgets.
 */
UCLASS()
class PROJECTIRONTABLE_API UTaskbar : public UUserWidget
{
	GENERATED_BODY()

public:

#pragma region Config
	/** The TaskbarButton class to instantiate for each registered widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UTaskbarButton> TaskbarButtonClass;
#pragma endregion

private:

#pragma region Widget References
	/** Horizontal container holding all taskbar buttons. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> ButtonContainer;

	/** Button the player clicks to reset all panels to their default sizes and positions. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResetButton;
#pragma endregion

protected:
	virtual void NativeConstruct() override;

public:
	/** Creates a TaskbarButton for the given widget and adds it to the taskbar. */
	UTaskbarButton* RegisterWidget(UUserWidget* Widget, FString Label);

	/** Resets all panels to their default sizes and positions */
	UFUNCTION()
	void ResetLayout();

};
