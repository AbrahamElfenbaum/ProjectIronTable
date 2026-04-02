// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MainScreenHUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

// Disables tick.
UMainScreenHUDComponent::UMainScreenHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Creates and adds the main screen widget, then caches button references and binds click handlers.
void UMainScreenHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerControllerRef = Cast<APlayerController>(GetOwner());

	if (PlayerControllerRef &&
		PlayerControllerRef->IsLocalPlayerController() &&
		MainScreenClass)
	{
		MainScreenRef = CreateWidget<UUserWidget>(GetWorld(), MainScreenClass);
		MainScreenRef->AddToViewport();

		PlayButtonRef = Cast<UButton>(MainScreenRef->GetWidgetFromName(TEXT("PlayButton")));
		JoinButtonRef = Cast<UButton>(MainScreenRef->GetWidgetFromName(TEXT("JoinButton")));
		LibraryButtonRef = Cast<UButton>(MainScreenRef->GetWidgetFromName(TEXT("LibraryButton")));
		SettingsButtonRef = Cast<UButton>(MainScreenRef->GetWidgetFromName(TEXT("SettingsButton")));
		QuitButtonRef = Cast<UButton>(MainScreenRef->GetWidgetFromName(TEXT("QuitButton")));

		if (PlayButtonRef)
		{
			PlayButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnPlayClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Play Button Not Found"));
		}

		if (JoinButtonRef)
		{
			JoinButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnJoinClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Join Button Not Found"));
		}

		if (LibraryButtonRef)
		{
			LibraryButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnLibraryClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Library Button Not Found"));
		}

		if (SettingsButtonRef)
		{
			SettingsButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnSettingsClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Settings Button Not Found"));
		}

		if (QuitButtonRef)
		{
			QuitButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnQuitClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Quit Button Not Found"));
		}
	}
}

// Opens the gameplay level.
void UMainScreenHUDComponent::OnPlayClicked()
{
	UGameplayStatics::OpenLevel(this, TEXT("L_Gameplay"));
}

// Placeholder — join session flow not yet implemented.
void UMainScreenHUDComponent::OnJoinClicked()
{
}

// Placeholder — asset library screen not yet implemented.
void UMainScreenHUDComponent::OnLibraryClicked()
{
}

// Placeholder — settings panel not yet implemented.
void UMainScreenHUDComponent::OnSettingsClicked()
{
}

// Quits the application.
void UMainScreenHUDComponent::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, PlayerControllerRef, EQuitPreference::Quit, false);
}
