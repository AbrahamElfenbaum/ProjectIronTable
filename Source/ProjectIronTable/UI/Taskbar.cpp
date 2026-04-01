// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "Taskbar.h"
#include "TaskbarButton.h"

// Creates a TaskbarButton for the widget, assigns it a label, and adds it to the button container.
UTaskbarButton* UTaskbar::RegisterWidget(UUserWidget* Widget, FString Label)
{
	UTaskbarButton* TaskbarButton = CreateWidget<UTaskbarButton>(GetOwningPlayer(), TaskbarButtonClass);
	TaskbarButton->SetTrackedWidget(Widget, Label);
	ButtonContainer->AddChild(TaskbarButton);
	return TaskbarButton;
}
