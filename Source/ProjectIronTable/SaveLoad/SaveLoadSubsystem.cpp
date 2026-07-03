// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SaveLoadSubsystem.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"

// Logs the ProductUserID on success or the error on failure when EOS login completes.
void USaveLoadSubsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserID, const FString& Error)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Display, TEXT("USaveLoadSubsystem::HandleLoginComplete - Logged in! ProductUserID: %s"), *UserID.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::HandleLoginComplete - Login FAILED: %s"), *Error);
	}
}

// Clears the write delegate handle and forwards the cloud-write result to the pending caller.
void USaveLoadSubsystem::HandleWriteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserID, const FString& FileName)
{
	IOnlineUserCloudPtr OUCP;
	FString CloudError;
	if (!GetUserCloud(TEXT("HandleWriteUserFileComplete"), OUCP, CloudError))
	{
		PendingCloudSave.ExecuteIfBound(false, CloudError);
		return;
	}

	OUCP->ClearOnWriteUserFileCompleteDelegate_Handle(WriteUserFileHandle);
	PendingCloudSave.ExecuteIfBound(bWasSuccessful, bWasSuccessful ? TEXT("") : TEXT("Cloud write failed."));
}

// Clears the read delegate handle, then creates a fresh save on a miss or deserializes the file bytes on success.
void USaveLoadSubsystem::HandleReadUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserID, const FString& FileName)
{
	IOnlineUserCloudPtr OUCP;
	FString CloudError;
	if (!GetUserCloud(TEXT("HandleReadUserFileComplete"), OUCP, CloudError))
	{
		PendingCloudLoad.ExecuteIfBound(false, nullptr, CloudError);
		return;
	}

	if (!bWasSuccessful)
	{
		OUCP->ClearOnReadUserFileCompleteDelegate_Handle(ReadUserFileHandle);

		USaveGame* LoadObject = UGameplayStatics::CreateSaveGameObject(PendingCloudLoadClass);
		if (!IsValid(LoadObject))
		{
			UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::HandleReadUserFileComplete - Failed to create save object."));
			PendingCloudLoad.ExecuteIfBound(false, nullptr, TEXT("Failed to create save object."));
			return;
		}

		PendingCloudLoad.ExecuteIfBound(true, LoadObject, TEXT(""));
		return;
	}

	OUCP->ClearOnReadUserFileCompleteDelegate_Handle(ReadUserFileHandle);
	if (!OUCP->GetFileContents(UserID, FileName, CloudSaveBuffer))
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::HandleReadUserFileComplete - Failed to get file contents."));
		PendingCloudLoad.ExecuteIfBound(false, nullptr, TEXT("Failed to get file contents."));
		return;
	}

	USaveGame* LoadObject = UGameplayStatics::LoadGameFromMemory(CloudSaveBuffer);
	if (!IsValid(LoadObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::HandleReadUserFileComplete - Failed to deserialize save."));
		PendingCloudLoad.ExecuteIfBound(false, nullptr, TEXT("Failed to deserialize save."));
		return;
	}

	PendingCloudLoad.ExecuteIfBound(bWasSuccessful, LoadObject, TEXT(""));
}

// Grabs the online subsystem and starts EOS login on game instance startup.
void USaveLoadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineSubsystem* OS = IOnlineSubsystem::Get();
	if (!OS)
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::Initialize - No online subsystem found. Check plugin + DefaultEngine.ini."));
		return;
	}

	IOnlineIdentityPtr OIP = OS->GetIdentityInterface();
	if (!OIP.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::Initialize - Identity interface is invalid."));
		return;
	}

	if (OIP->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		FUniqueNetIdPtr UserID = OIP->GetUniquePlayerId(0);
		if (UserID.IsValid())
		{
			UE_LOG(LogTemp, Display, TEXT("USaveLoadSubsystem::Initialize - Already logged in. ProductUserID: %s"), *UserID->ToString());
		}
		return;
	}

	LoginCompleteDelegateHandle = OIP->AddOnLoginCompleteDelegate_Handle(
		0,
		FOnLoginCompleteDelegate::CreateUObject(this, &USaveLoadSubsystem::HandleLoginComplete));

	FOnlineAccountCredentials OAC;
	OAC.Type = TEXT("accountportal");
	OAC.Id = TEXT("");
	OAC.Token = TEXT("");
	OIP->Login(0, OAC);

	UE_LOG(LogTemp, Display, TEXT("USaveLoadSubsystem::Initialize - Account portal login started, waiting for result."));
}

// Clears the login delegate handle on subsystem teardown.
void USaveLoadSubsystem::Deinitialize()
{
	Super::Deinitialize();

	IOnlineSubsystem* OS = IOnlineSubsystem::Get();
	if (OS == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::Deinitialize - No online subsystem found. Check plugin + DefaultEngine.ini."));
		return;
	}

	IOnlineIdentityPtr OIP = OS->GetIdentityInterface();
	if (!OIP.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::Deinitialize - Identity interface is invalid."));
		return;
	}

	OIP->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
}

// Saves the given save object to a local disk slot, then fires OnSaveComplete.
void USaveLoadSubsystem::SaveToLocal(USaveGame* Data, const FString& SlotName, const FOnSaveComplete& OnSaveComplete)
{
	if (!IsValid(Data))
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToLocal - Invalid save object."));
		OnSaveComplete.ExecuteIfBound(false, TEXT("Invalid save object."));
		return;
	}

	if (!UGameplayStatics::SaveGameToSlot(Data, SlotName, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToLocal - Failed to save game."));
		OnSaveComplete.ExecuteIfBound(false, TEXT("Failed to save game."));
		return;
	}

	OnSaveComplete.ExecuteIfBound(true, TEXT(""));
}

// Loads state from a local disk slot, creating a fresh object if none exists, then fires OnLoadComplete.
void USaveLoadSubsystem::LoadFromLocal(TSubclassOf<USaveGame> SaveClass, const FString& SlotName, const FOnLoadComplete& OnLoadComplete)
{
	USaveGame* LoadData = UGameplayStatics::LoadGameFromSlot(SlotName, 0);

	if (!IsValid(LoadData))
	{
		LoadData = UGameplayStatics::CreateSaveGameObject(SaveClass);
		if (!IsValid(LoadData))
		{
			UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::LoadFromLocal - Failed to create save object."));
			OnLoadComplete.ExecuteIfBound(false, nullptr, TEXT("Failed to create save object."));
			return;
		}
	}

	OnLoadComplete.ExecuteIfBound(true, LoadData, TEXT(""));
}

// Serializes the save object and starts an async write to EOS Player Data Storage; the completion handler fires OnSaveComplete.
void USaveLoadSubsystem::SaveToClient(USaveGame* Data, const FString& SlotName, const FOnSaveComplete& OnSaveComplete)
{
	WriteCloud(Data, SlotName, OnSaveComplete);
}

// Starts an async read of the save file from EOS Player Data Storage; the completion handler fires OnLoadComplete.
void USaveLoadSubsystem::LoadFromClient(TSubclassOf<USaveGame> SaveClass, const FString& SlotName, const FOnLoadComplete& OnLoadComplete)
{
	ReadCloud(SaveClass, SlotName, OnLoadComplete);
}

// Rejects client callers, then persists authority-owned shared state to the host's EOS Player Data Storage via WriteCloud.
void USaveLoadSubsystem::SaveToServer(USaveGame* Data, const FString& SlotName, const FOnSaveComplete& OnSaveComplete)
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToServer - No world."));
		OnSaveComplete.ExecuteIfBound(false, TEXT("No world."));
		return;
	}

	if (World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToServer - Rejected: clients cannot write server state."));
		OnSaveComplete.ExecuteIfBound(false, TEXT("Not authority."));
		return;
	}

	WriteCloud(Data, SlotName, OnSaveComplete);
}

// Rejects client callers, then loads authority-owned shared state from the host's EOS Player Data Storage via ReadCloud.
void USaveLoadSubsystem::LoadFromServer(TSubclassOf<USaveGame> SaveClass, const FString& SlotName, const FOnLoadComplete& OnLoadComplete)
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::LoadFromServer - No world."));
		OnLoadComplete.ExecuteIfBound(false, nullptr, TEXT("No world."));
		return;
	}

	if (World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::LoadFromServer - Rejected: clients cannot read server state."));
		OnLoadComplete.ExecuteIfBound(false, nullptr, TEXT("Not authority."));
		return;
	}

	ReadCloud(SaveClass, SlotName, OnLoadComplete);
}

// Resolves the online subsystem and user cloud interface; sets OutError and returns false on any failure without firing a delegate.
bool USaveLoadSubsystem::GetUserCloud(const TCHAR* Context, IOnlineUserCloudPtr& OutCloud, FString& OutError) const
{
	IOnlineSubsystem* OS = IOnlineSubsystem::Get();
	if (!OS)
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::%s - Invalid online subsystem."), Context);
		OutError = TEXT("Invalid online subsystem.");
		return false;
	}

	OutCloud = OS->GetUserCloudInterface();
	if (!OutCloud.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::%s - Invalid user cloud interface."), Context);
		OutError = TEXT("Invalid user cloud interface.");
		return false;
	}
	return true;
}

// Resolves the user cloud interface plus the local player's net id, reusing GetUserCloud; sets OutError and returns false on failure.
bool USaveLoadSubsystem::GetCloudAndUser(const TCHAR* Context, IOnlineUserCloudPtr& OutCloud, FUniqueNetIdPtr& OutUserID, FString& OutError) const
{
	if (!GetUserCloud(Context, OutCloud, OutError))
	{
		return false;
	}

	IOnlineIdentityPtr OIDP = IOnlineSubsystem::Get()->GetIdentityInterface();
	if (!OIDP.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::%s - Invalid identity interface."), Context);
		OutError = TEXT("Invalid identity interface.");
		return false;
	}

	OutUserID = OIDP->GetUniquePlayerId(0);
	if (!OutUserID.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::%s - Invalid user ID."), Context);
		OutError = TEXT("Invalid user ID.");
		return false;
	}

	return true;
}

// Serializes Data and starts an async write to the local user's EOS Player Data Storage; the completion handler fires OnSaveComplete.
void USaveLoadSubsystem::WriteCloud(USaveGame* Data, const FString& SlotName, FOnSaveComplete OnSaveComplete)
{
	IOnlineUserCloudPtr OUCP;
	FUniqueNetIdPtr UID;
	FString CloudError;
	if (!GetCloudAndUser(TEXT("WriteCloud"), OUCP, UID, CloudError))
	{
		OnSaveComplete.ExecuteIfBound(false, CloudError);
		return;
	}

	if (!IsValid(Data))
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::WriteCloud - Invalid save object."));
		OnSaveComplete.ExecuteIfBound(false, TEXT("Invalid save object."));
		return;
	}

	if (!UGameplayStatics::SaveGameToMemory(Data, CloudSaveBuffer))
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::WriteCloud - Save to memory failed."));
		OnSaveComplete.ExecuteIfBound(false, TEXT("Save to memory failed."));
		return;
	}

	WriteUserFileHandle = OUCP->AddOnWriteUserFileCompleteDelegate_Handle(
		FOnWriteUserFileCompleteDelegate::CreateUObject(this, &USaveLoadSubsystem::HandleWriteUserFileComplete));

	PendingCloudSave = OnSaveComplete;

	if (!OUCP->WriteUserFile(*UID, SlotName, CloudSaveBuffer))
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::WriteCloud - Write did not start."));
		OUCP->ClearOnWriteUserFileCompleteDelegate_Handle(WriteUserFileHandle);
		OnSaveComplete.ExecuteIfBound(false, TEXT("Write did not start."));
		return;
	}
}

// Starts an async read from the local user's EOS Player Data Storage; the completion handler fires OnLoadComplete.
void USaveLoadSubsystem::ReadCloud(TSubclassOf<USaveGame> SaveClass, const FString& SlotName, FOnLoadComplete OnLoadComplete)
{
	IOnlineUserCloudPtr OUCP;
	FUniqueNetIdPtr UID;
	FString CloudError;
	if (!GetCloudAndUser(TEXT("ReadCloud"), OUCP, UID, CloudError))
	{
		OnLoadComplete.ExecuteIfBound(false, nullptr, CloudError);
		return;
	}

	ReadUserFileHandle = OUCP->AddOnReadUserFileCompleteDelegate_Handle(
		FOnReadUserFileCompleteDelegate::CreateUObject(this, &USaveLoadSubsystem::HandleReadUserFileComplete));

	PendingCloudLoadClass = SaveClass;
	PendingCloudLoad = OnLoadComplete;

	if (!OUCP->ReadUserFile(*UID, SlotName))
	{
		UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::ReadCloud - Read did not start."));
		OUCP->ClearOnReadUserFileCompleteDelegate_Handle(ReadUserFileHandle);
		OnLoadComplete.ExecuteIfBound(false, nullptr, TEXT("Read did not start."));
		return;
	}
}
