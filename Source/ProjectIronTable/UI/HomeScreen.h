// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HomeScreen.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJoinRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLibraryRequested);

/** Home screen widget. Binds all home screen buttons and broadcasts delegates for navigation events that require parent coordination. */
UCLASS()
class PROJECTIRONTABLE_API UHomeScreen : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Binds all button click delegates. */
	void Init();

private:

#pragma region Buttons
	/** Button that transitions to the gameplay scene. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayButton;

	/** Button that opens the join session flow (not yet implemented). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	/** Button that opens the asset library screen (not yet implemented). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LibraryButton;

	/** Button that opens the settings panel. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SettingsButton;

	/** Button that quits the application. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;
#pragma endregion

public:

	/** Broadcasts when the user requests to open the settings screen. */
	UPROPERTY(BlueprintAssignable)
	FOnSettingsRequested OnSettingsRequested;

	/** Broadcasts when the user requests to join a session. */
	UPROPERTY(BlueprintAssignable)
	FOnJoinRequested OnJoinRequested;

	/** Broadcasts when the user requests to open the asset library. */
	UPROPERTY(BlueprintAssignable)
	FOnLibraryRequested OnLibraryRequested;

private:

#pragma region Event Functions
	/** Opens the gameplay level. */
	UFUNCTION()
	void OnPlayClicked();

	/** Broadcasts OnJoinRequested. */
	UFUNCTION()
	void OnJoinClicked();

	/** Broadcasts OnLibraryRequested. */
	UFUNCTION()
	void OnLibraryClicked();

	/** Opens the settings screen. */
	UFUNCTION()
	void OnSettingsClicked();

	/** Quits the application. */
	UFUNCTION()
	void OnQuitClicked();
#pragma endregion


};
