// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DiceData.h"
#include "Blueprint/UserWidget.h"
#include "FunctionLibrary.generated.h"

/** General-purpose static utility functions exposed to Blueprints. */
UCLASS()
class PROJECTIRONTABLE_API UFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the display name string for a given EDiceType (e.g. "D20"). */
	UFUNCTION(BlueprintPure, Category = "Dice")
	static FString GetDiceName(EDiceType Type);

	/** Finds a named child widget within the given UserWidget and casts it to T. Returns nullptr and logs a warning if the widget is not found or the cast fails. */
	template<typename T>
	static T* GetTypedWidgetFromName(UUserWidget* Widget, FName Name)
	{
		if (!Widget)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetTypedWidgetFromName: Widget is null"));
			return nullptr;
		}
		T* Result = Cast<T>(Widget->GetWidgetFromName(Name));
		if (!Result)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetTypedWidgetFromName: Failed to find or cast widget '%s'"), *Name.ToString());
		}
		return Result;
	}
};