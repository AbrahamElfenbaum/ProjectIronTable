// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "BaseScreen.h"

#include "Components/Button.h"

// Broadcasts OnBackRequested to signal the parent to return to the home screen.
void UBaseScreen::OnBackClicked()
{
	OnBackRequested.Broadcast();
}

// Binds the back button click delegate.
void UBaseScreen::NativeConstruct()
{
	Super::NativeConstruct();
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UBaseScreen::OnBackClicked);
	}
}
