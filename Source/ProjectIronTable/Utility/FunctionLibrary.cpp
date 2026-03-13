#include "FunctionLibrary.h"

FString UFunctionLibrary::GetDiceName(EDiceType Type)
{
	switch (Type)
	{
	case EDiceType::D4:
		return "D4";
	case EDiceType::D6:
		return "D6";
	case EDiceType::D8:
		return "D8";
	case EDiceType::D10:
		return "D10";
	case EDiceType::D12:
		return "D12";
	case EDiceType::D20:
		return "D20";
	case EDiceType::D100:
		return "D100";
	default:
		return FString();
	}
}