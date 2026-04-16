// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "TaskbarButton.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "MacroLibrary.h"

// Binds the toggle button click event.
void UTaskbarButton::NativeConstruct()
{
	Super::NativeConstruct();
	if (ToggleButton)
	{
		ToggleButton->OnClicked.AddDynamic(this, &UTaskbarButton::OnToggleClicked);
	}
}

// Stores the widget reference and sets the label text.
void UTaskbarButton::SetTrackedWidget(UUserWidget* Widget, const FString& Label)
{
	TrackedWidget = Widget;
	if (WidgetLabel)
	{
		WidgetLabel->SetText(FText::FromString(Label));
	}
}

// Returns the widget being tracked by this button.
UUserWidget* UTaskbarButton::GetTrackedWidget() const
{
	return TrackedWidget;
}

// Collapses the tracked widget if visible, or restores it if collapsed.
void UTaskbarButton::OnToggleClicked()
{
	CHECK_IF_VALID(TrackedWidget, );

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
