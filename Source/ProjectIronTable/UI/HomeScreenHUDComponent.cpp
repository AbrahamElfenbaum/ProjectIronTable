// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "HomeScreenHUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

// Disables tick.
UHomeScreenHUDComponent::UHomeScreenHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Creates and adds the home screen widget, then caches button references and binds click handlers.
void UHomeScreenHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerControllerRef = Cast<APlayerController>(GetOwner());

	if (PlayerControllerRef &&
		PlayerControllerRef->IsLocalPlayerController() &&
		HomeScreenClass)
	{
		HomeScreenRef = CreateWidget<UUserWidget>(GetWorld(), HomeScreenClass);
		HomeScreenRef->AddToViewport();

		PlayButtonRef = Cast<UButton>(HomeScreenRef->GetWidgetFromName(TEXT("PlayButton")));
		JoinButtonRef = Cast<UButton>(HomeScreenRef->GetWidgetFromName(TEXT("JoinButton")));
		LibraryButtonRef = Cast<UButton>(HomeScreenRef->GetWidgetFromName(TEXT("LibraryButton")));
		SettingsButtonRef = Cast<UButton>(HomeScreenRef->GetWidgetFromName(TEXT("SettingsButton")));
		QuitButtonRef = Cast<UButton>(HomeScreenRef->GetWidgetFromName(TEXT("QuitButton")));

		if (PlayButtonRef)
		{
			PlayButtonRef->OnClicked.AddDynamic(this, &UHomeScreenHUDComponent::OnPlayClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Play Button Not Found"));
		}

		if (JoinButtonRef)
		{
			JoinButtonRef->OnClicked.AddDynamic(this, &UHomeScreenHUDComponent::OnJoinClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Join Button Not Found"));
		}

		if (LibraryButtonRef)
		{
			LibraryButtonRef->OnClicked.AddDynamic(this, &UHomeScreenHUDComponent::OnLibraryClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Library Button Not Found"));
		}

		if (SettingsButtonRef)
		{
			SettingsButtonRef->OnClicked.AddDynamic(this, &UHomeScreenHUDComponent::OnSettingsClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Settings Button Not Found"));
		}

		if (QuitButtonRef)
		{
			QuitButtonRef->OnClicked.AddDynamic(this, &UHomeScreenHUDComponent::OnQuitClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Quit Button Not Found"));
		}
	}
}

// Opens the gameplay level.
void UHomeScreenHUDComponent::OnPlayClicked()
{
	UGameplayStatics::OpenLevel(this, TEXT("L_Gameplay"));
}

// Placeholder — join session flow not yet implemented.
void UHomeScreenHUDComponent::OnJoinClicked()
{
}

// Placeholder — asset library screen not yet implemented.
void UHomeScreenHUDComponent::OnLibraryClicked()
{
}

// Placeholder — settings panel not yet implemented.
void UHomeScreenHUDComponent::OnSettingsClicked()
{
}

// Quits the application.
void UHomeScreenHUDComponent::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, PlayerControllerRef, EQuitPreference::Quit, false);
}
