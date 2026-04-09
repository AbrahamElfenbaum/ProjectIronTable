// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "PlayerRow.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

// Binds the address button click delegate.
void UPlayerRow::NativeConstruct()
{
	Super::NativeConstruct();
	AddressButton->OnClicked.AddDynamic(this, &UPlayerRow::OnAddressButtonClicked);
}

// Stores the name and updates the name label text.
void UPlayerRow::SetPlayerName(const FString& Name)
{
	PlayerName = Name;
	if (NameLabel)
	{
		NameLabel->SetText(FText::FromString(Name));
	}
}

// Broadcasts OnAddressClicked with the stored player name.
void UPlayerRow::OnAddressButtonClicked()
{
	OnAddressClicked.Broadcast(PlayerName);
}
