// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MainScreenHUDComponent.generated.h"

class UWidgetSwitcher;
class UHomeScreen;
class USettingsScreen;

/** Actor component attached to AMainScreenController that owns and manages all main screen UI. */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API UMainScreenHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Disables tick. */
	UMainScreenHUDComponent();

protected:
	/** Creates and adds the main screen widget, then caches widget references and binds click handlers. */
	virtual void BeginPlay() override;

public:

#pragma region Config
	/** The root main screen widget class to instantiate and add to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> MainScreenClass;
#pragma endregion

#pragma region Main Screen
	/** The instantiated root main screen widget. */
	UPROPERTY()
	TObjectPtr<UUserWidget> MainScreenRef;

	/** The instantiated screen switcher widget. */
	UPROPERTY()
	TObjectPtr<UWidgetSwitcher> ScreenSwitcherRef;
#pragma endregion

#pragma region Home Screen
	/** The instantiated home screen widget. */
	UPROPERTY()
	TObjectPtr<UHomeScreen> HomeScreenRef;

	/** Cached reference to the owning player controller. */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerControllerRef;
#pragma endregion

	/** The instantiated settings screen widget. */
	UPROPERTY()
	TObjectPtr<USettingsScreen> SettingsScreenRef;

private:

#pragma region Event Handlers
	/** Switches the screen switcher back to the home screen. */
	UFUNCTION()
	void OnBackClicked();
#pragma endregion

#pragma region Main Screen
	/** Switches the screen switcher to the settings screen. */
	UFUNCTION()
	void OnSettingsClicked();
#pragma endregion
};
