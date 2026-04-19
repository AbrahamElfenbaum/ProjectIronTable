// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "BaseChannelPanel.h"
#include "BaseChannel.h"
#include "BaseChannelTab.h"
#include "MacroLibrary.h"
#include "Components/WidgetSwitcher.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "FunctionLibrary.h"
#include "SessionInstance.h"
#include "SessionSave.h"
#include "Kismet/GameplayStatics.h"

//
void UBaseChannelPanel::NativeConstruct()
{
	Super::NativeConstruct();
}

//
FReply UBaseChannelPanel::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	return FReply::Handled();
}

//
void UBaseChannelPanel::Scroll(bool bUp)
{
	if (IsValid(ActiveChannel)) ActiveChannel->Scroll(bUp);
}

//
UBaseChannel* UBaseChannelPanel::CreateChannel(const TArray<FString>& Participants)
{
	return nullptr;
}

//
TArray<FString>  UBaseChannelPanel::GetActiveChannelParticipants()
{
	return {};
}

//
UBaseChannel* UBaseChannelPanel::FindOrCreateChannel(const TArray<FString>& Participants)
{
	return nullptr;
}

//
UBaseChannelTab* UBaseChannelPanel::GetTabForChannel(UBaseChannel* Channel) const
{
	return nullptr;
}

//
void UBaseChannelPanel::SwitchToChannel(UBaseChannel* Channel)
{

}

//
void UBaseChannelPanel::OnChannelListButtonClicked()
{

}

//
void UBaseChannelPanel::CloseChannel(UBaseChannel* Channel)
{

}

//
void UBaseChannelPanel::ReopenChannel(UBaseChannel* Channel)
{

}

//
void UBaseChannelPanel::OnTabRightClickedHandler(UBaseChannel* Channel)
{

}

//
void UBaseChannelPanel::OnTabRenamedHandler(UBaseChannelTab* Tab, const FString& NewName)
{

}

//
void UBaseChannelPanel::RefreshChannelList()
{

}