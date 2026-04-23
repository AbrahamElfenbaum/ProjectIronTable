// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DelegateLibrary.h"
#include "BaseScreen.generated.h"

class UButton;

/** Base class for all main screen widgets. Provides a shared back button, OnBackRequested delegate, and a virtual Init hook. */
UCLASS()
class PROJECTIRONTABLE_API UBaseScreen : public UUserWidget
{
	GENERATED_BODY()

public:

#pragma region Events
	/** Fired when the back button is clicked; parent should return to the home screen. */
	UPROPERTY(BlueprintAssignable)
	FOnBackRequested OnBackRequested;
#pragma endregion

#pragma region Public Methods
	/** Override in subclasses to perform screen-specific setup. */
	virtual void Init() {}
#pragma endregion

protected:
	/** Binds the back button click delegate. */
	virtual void NativeConstruct() override;

private:

#pragma region Widget References
	/** Button that returns to the home screen. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;
#pragma endregion

#pragma region Event Handlers
	/** Broadcasts OnBackRequested to signal the parent to return to the home screen. */
	UFUNCTION()
	void OnBackClicked();
#pragma endregion
};
