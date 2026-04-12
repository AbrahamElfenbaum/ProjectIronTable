// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionInstance.h"

// Return the stored campaign ID
FGuid USessionInstance::GetCampaignID() const
{
	return CampaignID;
}

// Return the stored session ID
FGuid USessionInstance::GetSessionID() const
{
	return SessionID;
}

// Store the campaign ID for use after level transition
void USessionInstance::SetCampaignID(const FGuid& InCampaignID)
{
	CampaignID = InCampaignID;
}

// Store the session ID for use after level transition
void USessionInstance::SetSessionID(const FGuid& InSessionID)
{
	SessionID = InSessionID;
}