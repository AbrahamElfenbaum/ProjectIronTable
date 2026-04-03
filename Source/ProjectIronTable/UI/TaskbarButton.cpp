// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "TaskbarButton.h"

// Binds the toggle button click event.
void UTaskbarButton::NativeConstruct()
{
	Super::NativeConstruct();
	// -- Bind Events --
	if (ToggleButton)
	{
		ToggleButton->OnClicked.AddDynamic(this, &UTaskbarButton::OnToggleClicked);
	}
}

// Stores the widget reference and sets the label text.
void UTaskbarButton::SetTrackedWidget(UUserWidget* Widget, FString Label)
{
	TrackedWidget = Widget;
	WidgetLabel->SetText(FText::FromString(Label));
}

UUserWidget* UTaskbarButton::GetTrackedWidget() const
{
	return TrackedWidget;
}

// Collapses the tracked widget if visible, or restores it if collapsed.
void UTaskbarButton::OnToggleClicked()
{
	if (TrackedWidget->GetVisibility() == ESlateVisibility::Collapsed)
	{
		TrackedWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		TrackedWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	OnToggled.Broadcast();
}
