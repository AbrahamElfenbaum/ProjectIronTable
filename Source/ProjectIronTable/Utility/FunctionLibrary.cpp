// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.

#include "FunctionLibrary.h"

// Returns the short string name for each EDiceType value.
FString UFunctionLibrary::GetDiceName(EDiceType Type)
{
	switch (Type)
	{
	case EDiceType::D4:
		return TEXT("D4");
	case EDiceType::D6:
		return TEXT("D6");
	case EDiceType::D8:
		return TEXT("D8");
	case EDiceType::D10:
		return TEXT("D10");
	case EDiceType::D12:
		return TEXT("D12");
	case EDiceType::D20:
		return TEXT("D20");
	case EDiceType::D100:
		return TEXT("D100");
	default:
		return FString();
	}
}
