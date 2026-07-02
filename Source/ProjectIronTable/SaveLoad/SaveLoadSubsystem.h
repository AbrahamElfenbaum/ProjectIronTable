// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/SubclassOf.h"
#include "SaveLoadSubsystem.generated.h"

class USaveGame;

DECLARE_DELEGATE_TwoParams(FOnSaveComplete, bool, const FString&);
DECLARE_DELEGATE_ThreeParams(FOnLoadComplete, bool, USaveGame*, const FString&);

/**
 * GameInstance subsystem that acts as the single save/load facade. Routes persistence to the
 * appropriate backend (local disk, EOS Player Data Storage, or the authority) behind a uniform
 * async delegate contract, and owns EOS login.
 */
UCLASS()
class PROJECTIRONTABLE_API USaveLoadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	/** Class of the save object to create when a cloud load finds no existing file. */
	TSubclassOf<USaveGame> PendingCloudLoadClass;

	/** Serialized save bytes kept alive for the duration of the async cloud write/read. */
	TArray<uint8> CloudSaveBuffer;

	/** Caller's completion delegate, stashed across the async cloud write until EOS calls back. */
	FOnSaveComplete PendingCloudSave;

	/** Caller's completion delegate, stashed across the async cloud read until EOS calls back. */
	FOnLoadComplete PendingCloudLoad;

	/** Handle for the registered login completion delegate; cleared in Deinitialize. */
	FDelegateHandle LoginCompleteDelegateHandle;

	/** Handle for the registered cloud-write completion delegate; cleared once the write finishes. */
	FDelegateHandle WriteUserFileHandle;

	/** Handle for the registered cloud-read completion delegate; cleared once the read finishes. */
	FDelegateHandle ReadUserFileHandle;

	/** Logs the ProductUserId on success or the error on failure when EOS login completes. */
	void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	/** Clears the write delegate handle and forwards the cloud-write result to the pending caller. */
	void HandleWriteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);

	/** Clears the read delegate handle, then creates a fresh save on a miss or deserializes the file bytes on success, and forwards the result. */
	void HandleReadUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);

protected:
	/** Grabs the online subsystem and starts EOS login on game instance startup. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Clears the login delegate handle on subsystem teardown. */
	virtual void Deinitialize() override;

public:
	/** Saves the given save object to a local disk slot, then fires OnSaveComplete. */
	void SaveToLocal(USaveGame* Data, const FString& SlotName, const FOnSaveComplete& OnSaveComplete);

	/** Loads state from a local disk slot (creating a fresh object if none exists), then fires OnLoadComplete. */
	void LoadFromLocal(TSubclassOf<USaveGame> SaveClass, const FString& SlotName, const FOnLoadComplete& OnLoadComplete);

	/** Serializes state and writes it to EOS Player Data Storage, firing OnSaveComplete when the async write returns. */
	void SaveToClient(USaveGame* Data, const FString& SlotName, const FOnSaveComplete& OnSaveComplete);

	/** Reads state from EOS Player Data Storage (creating a fresh object if the file is not found), firing OnLoadComplete with the result. */
	void LoadFromClient(TSubclassOf<USaveGame> SaveClass, const FString& SlotName, const FOnLoadComplete& OnLoadComplete);

	/** Persists authority-owned shared state. Not yet implemented (listen-host backend pending). */
	void SaveToServer(USaveGame* Data, const FString& SlotName, const FOnSaveComplete& OnSaveComplete);

	/** Loads authority-owned shared state. Not yet implemented (listen-host backend pending). */
	void LoadFromServer(TSubclassOf<USaveGame> SaveClass, const FString& SlotName, const FOnLoadComplete& OnLoadComplete);
};
