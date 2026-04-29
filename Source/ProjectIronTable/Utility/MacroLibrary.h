// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once

/** Retrieves the typed outer of Type, assigns it to VarName, and returns ReturnVal if the outer is invalid. */
#define GET_OUTER(Type, VarName, ReturnVal) \
        Type* VarName = GetTypedOuter<Type>(); \
        if (!IsValid(VarName)) { UE_LOG(LogTemp, Warning, TEXT("%hs — outer " #Type " is null"), __FUNCTION__); return ReturnVal; }

/** Casts GetOwningPlayer() to APlayerController, assigns it to VarName, and returns ReturnVal if the cast or validity check fails. */
#define GET_OWNING_PC(VarName, ReturnVal) \
      APlayerController* VarName = Cast<APlayerController>(GetOwningPlayer()); \
      if (!IsValid(VarName)) { UE_LOG(LogTemp, Warning, TEXT("%hs — owning PlayerController is null"), __FUNCTION__); return ReturnVal; }

/** Checks IsValid on VarName and returns ReturnVal with a warning log if the check fails. */
#define CHECK_IF_VALID(VarName, ReturnVal) \
      if (!IsValid(VarName)) { UE_LOG(LogTemp, Warning, TEXT("%hs — " #VarName " is null"), __FUNCTION__); return ReturnVal; }
