// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionNotesComponent.h"

#include "FunctionLibrary.h"
#include "MacroLibrary.h"

// Casts the owner to APlayerController and caches the reference.
void USessionNotesComponent::Init()
{
	PlayerControllerRef = Cast<APlayerController>(GetOwner());

	CHECK_IF_VALID(PlayerControllerRef, );
}

// Stub — logs not yet implemented.
void USessionNotesComponent::Server_PushNote_Implementation(FNoteRecord InRecord)
{
	UE_LOG(LogTemp, Warning, TEXT("USessionNotesComponent::Server_PushNote - Not yet implemented"));
}

// Stub — logs not yet implemented.
void USessionNotesComponent::Multicast_ReceiveNote_Implementation(FNoteRecord InRecord)
{
	UE_LOG(LogTemp, Warning, TEXT("USessionNotesComponent::Multicast_ReceiveNote - Not yet implemented"));
}

// Stub — logs not yet implemented.
void USessionNotesComponent::Server_RequestNoteSync_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("USessionNotesComponent::Server_RequestNoteSync - Not yet implemented"));
}
