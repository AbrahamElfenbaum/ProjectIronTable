// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "PlayerRow.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

// Stores the name and updates the name label text.
void UPlayerRow::SetPlayerName(const FString& Name)
{
	PlayerName = Name;
	if (NameLabel)
	{
		NameLabel->SetText(FText::FromString(Name));
	}
}

// Binds the address button click delegate.
void UPlayerRow::NativeConstruct()
{
	Super::NativeConstruct();
	if (AddressButton)
	{
		AddressButton->OnClicked.AddDynamic(this, &UPlayerRow::OnAddressButtonClicked);
	}
}

// Broadcasts OnAddressClicked with the stored player name.
void UPlayerRow::OnAddressButtonClicked()
{
	OnAddressClicked.Broadcast(PlayerName);
}
