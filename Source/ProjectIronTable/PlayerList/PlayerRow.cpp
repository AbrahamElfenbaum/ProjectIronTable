// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "PlayerRow.h"

void UPlayerRow::NativeConstruct()
{
	Super::NativeConstruct();
	AddressButton->OnClicked.AddDynamic(this, &UPlayerRow::OnAddressButtonClicked);
}

void UPlayerRow::SetPlayerName(const FString& Name)
{
	PlayerName = Name;
	if (NameLabel)
	{
		NameLabel->SetText(FText::FromString(Name));
	}
}

void UPlayerRow::OnAddressButtonClicked()
{
	OnAddressClicked.Broadcast(PlayerName);
}