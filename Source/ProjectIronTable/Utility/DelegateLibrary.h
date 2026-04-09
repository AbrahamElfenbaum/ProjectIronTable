// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DelegateLibrary.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackRequested);

/** Shared delegate type declarations used across multiple screens and systems. */
UCLASS()
class PROJECTIRONTABLE_API UDelegateLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};
