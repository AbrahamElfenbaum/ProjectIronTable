// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SaveLoadSubsystem.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"

// Logs the ProductUserId on success or the error on failure when EOS login completes.
void USaveLoadSubsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Display, TEXT("USaveLoadSubsystem::HandleLoginComplete - Logged in! ProductUserId: %s"), *UserId.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::HandleLoginComplete - Login FAILED: %s"), *Error);
    }
}

// Clears the write delegate handle and forwards the cloud-write result to the pending caller.
void USaveLoadSubsystem::HandleWriteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
    IOnlineSubsystem* OS = IOnlineSubsystem::Get();
    if (!OS)
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::HandleWriteUserFileComplete - Invalid online subsystem."));
        PendingCloudSave.ExecuteIfBound(false, TEXT("Invalid online subsystem."));
        return;
    }

    IOnlineUserCloudPtr OUCP = OS->GetUserCloudInterface();
    if (!OUCP.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::HandleWriteUserFileComplete - Invalid user cloud interface."));
        PendingCloudSave.ExecuteIfBound(false, TEXT("Invalid user cloud interface."));
        return;
    }

    OUCP->ClearOnWriteUserFileCompleteDelegate_Handle(WriteUserFileHandle);
    PendingCloudSave.ExecuteIfBound(bWasSuccessful, bWasSuccessful ? TEXT("") : TEXT("Cloud write failed."));
}

// Clears the read delegate handle, then creates a fresh save on a miss or deserializes the file bytes on success.
void USaveLoadSubsystem::HandleReadUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
    IOnlineSubsystem* OS = IOnlineSubsystem::Get();
    if (!OS)
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::HandleReadUserFileComplete - Invalid online subsystem."));
        PendingCloudLoad.ExecuteIfBound(false, nullptr, TEXT("Invalid online subsystem."));
        return;
    }

    IOnlineUserCloudPtr OUCP = OS->GetUserCloudInterface();
    if (!OUCP.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::HandleReadUserFileComplete - Invalid user cloud interface."));
        PendingCloudLoad.ExecuteIfBound(false, nullptr, TEXT("Invalid user cloud interface."));
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
    if (!OUCP->GetFileContents(UserId, FileName, CloudSaveBuffer))
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
        FUniqueNetIdPtr UserId = OIP->GetUniquePlayerId(0);
        if (UserId.IsValid())
        {
            UE_LOG(LogTemp, Display, TEXT("USaveLoadSubsystem::Initialize - Already logged in. ProductUserId: %s"), *UserId->ToString());
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
    IOnlineSubsystem* OS = IOnlineSubsystem::Get();
    if (!OS)
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToClient - Invalid online subsystem."));
        OnSaveComplete.ExecuteIfBound(false, TEXT("Invalid online subsystem."));
        return;
    }

    IOnlineUserCloudPtr OUCP = OS->GetUserCloudInterface();
    if (!OUCP.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToClient - Invalid user cloud interface."));
        OnSaveComplete.ExecuteIfBound(false, TEXT("Invalid user cloud interface."));
        return;
    }

    IOnlineIdentityPtr OIDP = OS->GetIdentityInterface();
    if (!OIDP.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToClient - Invalid identity interface."));
        OnSaveComplete.ExecuteIfBound(false, TEXT("Invalid identity interface."));
        return;
    }

    FUniqueNetIdPtr UID = OIDP->GetUniquePlayerId(0);
    if (!UID.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToClient - Invalid user ID."));
        OnSaveComplete.ExecuteIfBound(false, TEXT("Invalid user ID."));
        return;
    }

    if (!IsValid(Data))
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToClient - Invalid save object."));
        OnSaveComplete.ExecuteIfBound(false, TEXT("Invalid save object."));
        return;
    }
    
    if (!UGameplayStatics::SaveGameToMemory(Data, CloudSaveBuffer))
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToClient - Save to memory failed."));
        OnSaveComplete.ExecuteIfBound(false, TEXT("Save to memory failed."));
        return;
    }

    WriteUserFileHandle = OUCP->AddOnWriteUserFileCompleteDelegate_Handle(
        FOnWriteUserFileCompleteDelegate::CreateUObject(this, &USaveLoadSubsystem::HandleWriteUserFileComplete));

    PendingCloudSave = OnSaveComplete;

    if (!OUCP->WriteUserFile(*UID, SlotName, CloudSaveBuffer))
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToClient - Write did not start."));
        OUCP->ClearOnWriteUserFileCompleteDelegate_Handle(WriteUserFileHandle);
        OnSaveComplete.ExecuteIfBound(false, TEXT("Write did not start."));
        return;
    }
}

// Starts an async read of the save file from EOS Player Data Storage; the completion handler fires OnLoadComplete.
void USaveLoadSubsystem::LoadFromClient(TSubclassOf<USaveGame> SaveClass, const FString& SlotName, const FOnLoadComplete& OnLoadComplete)
{
    IOnlineSubsystem* OS = IOnlineSubsystem::Get();
    if (!OS)
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::LoadFromClient - Invalid online subsystem."));
        OnLoadComplete.ExecuteIfBound(false, nullptr, TEXT("Invalid online subsystem."));
        return;
    }

    IOnlineUserCloudPtr OUCP = OS->GetUserCloudInterface();
    if (!OUCP.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::LoadFromClient - Invalid user cloud interface."));
        OnLoadComplete.ExecuteIfBound(false, nullptr, TEXT("Invalid user cloud interface."));
        return;
    }

    IOnlineIdentityPtr OIDP = OS->GetIdentityInterface();
    if (!OIDP.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::LoadFromClient - Invalid identity interface."));
        OnLoadComplete.ExecuteIfBound(false, nullptr, TEXT("Invalid identity interface."));
        return;
    }

    FUniqueNetIdPtr UID = OIDP->GetUniquePlayerId(0);
    if (!UID.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::LoadFromClient - Invalid user ID."));
        OnLoadComplete.ExecuteIfBound(false, nullptr, TEXT("Invalid user ID."));
        return;
    }

    ReadUserFileHandle = OUCP->AddOnReadUserFileCompleteDelegate_Handle(
        FOnReadUserFileCompleteDelegate::CreateUObject(this, &USaveLoadSubsystem::HandleReadUserFileComplete));

    PendingCloudLoadClass = SaveClass;
    PendingCloudLoad = OnLoadComplete;

    if (!OUCP->ReadUserFile(*UID, SlotName))
    {
        UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::LoadFromClient - Read did not start."));
        OUCP->ClearOnReadUserFileCompleteDelegate_Handle(ReadUserFileHandle);
        OnLoadComplete.ExecuteIfBound(false, nullptr, TEXT("Read did not start."));
        return;
    }
}

// Persists authority-owned shared state. Not yet implemented (listen-host backend pending).
void USaveLoadSubsystem::SaveToServer(USaveGame* Data, const FString& SlotName, const FOnSaveComplete& OnSaveComplete)
{
    UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::SaveToServer - not implemented."));
    OnSaveComplete.ExecuteIfBound(false, TEXT("SaveToServer not implemented."));
}

// Loads authority-owned shared state. Not yet implemented (listen-host backend pending).
void USaveLoadSubsystem::LoadFromServer(TSubclassOf<USaveGame> SaveClass, const FString& SlotName, const FOnLoadComplete& OnLoadComplete)
{
    UE_LOG(LogTemp, Warning, TEXT("USaveLoadSubsystem::LoadFromServer - not implemented."));
    OnLoadComplete.ExecuteIfBound(false, nullptr, TEXT("LoadFromServer not implemented."));
}
