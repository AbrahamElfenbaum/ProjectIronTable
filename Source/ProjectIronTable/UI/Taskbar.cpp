// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "Taskbar.h"

#include "Components/HorizontalBox.h"
#include "Components/Button.h"

#include "TaskbarButton.h"
#include "DraggablePanel.h"
#include "MacroLibrary.h"

// Binds the reset button click event.
void UTaskbar::NativeConstruct()
{
	Super::NativeConstruct();
	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UTaskbar::ResetLayout);
	}
}

// Creates a TaskbarButton for the widget, assigns it a label, and adds it to the button container.
UTaskbarButton* UTaskbar::RegisterWidget(UUserWidget* Widget, const FString& Label)
{
	UTaskbarButton* TaskbarButton = CreateWidget<UTaskbarButton>(GetOwningPlayer(), TaskbarButtonClass);
	CHECK_IF_VALID(TaskbarButton, nullptr);
	TaskbarButton->SetTrackedWidget(Widget, Label);
	ButtonContainer->AddChild(TaskbarButton);
	return TaskbarButton;
}

// Resets all panels to their default sizes and positions by broadcasting a message to all taskbar buttons.
void UTaskbar::ResetLayout()
{
	for (UWidget* ButtonWidget : ButtonContainer->GetAllChildren())
	{
		if (UTaskbarButton* TaskbarButton = Cast<UTaskbarButton>(ButtonWidget))
		{
			if (UDraggablePanel* Panel = Cast<UDraggablePanel>(TaskbarButton->GetTrackedWidget()))
			{
				Panel->ResetToDefaultLayout();
			}
		}
	}
}
