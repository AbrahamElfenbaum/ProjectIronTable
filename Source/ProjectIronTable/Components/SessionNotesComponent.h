// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SessionNotesSave.h"
#include "SessionNotesComponent.generated.h"

/**
 * Actor component attached to ASessionController that relays shared notes between players.
 * Holds an in-memory cache of shared notes on the server. RPC bodies are stubs — real-time sync is a later task.
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API USessionNotesComponent : public UActorComponent
{
	GENERATED_BODY()

private:

#pragma region Runtime References
	/** Cached reference to the owning player controller. */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerControllerRef;
#pragma endregion

#pragma region State
	/** In-memory relay cache of shared note records on the server. Not persisted — relay only. */
	TMap<FGuid, FNoteRecord> SharedNoteCache;
#pragma endregion

public:

	/** Caches the owning player controller reference. Must be called immediately after the component is created. */
	void Init();

	/** Server RPC — receives an updated note from a client and stores it in the relay cache. Not yet implemented. */
	UFUNCTION(Server, Reliable)
	void Server_PushNote(FNoteRecord InRecord);

	/** Multicast RPC — broadcasts a note update from the server to all connected clients. Not yet implemented. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReceiveNote(FNoteRecord InRecord);

	/** Server RPC — called by a reconnecting client to request the current shared note state from the relay cache. Not yet implemented. */
	UFUNCTION(Server, Reliable)
	void Server_RequestNoteSync();
};
