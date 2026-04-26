# ProjectIronTable — Coding Standards

## 1. File Header

Every `.h` and `.cpp` file must begin with the copyright line:

```cpp
// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
```

---

## 2. Include Discipline

- Full `#include` statements belong in `.cpp` files only.
- `.h` files use **forward declarations** for any type that appears only as a pointer or reference.
- The only exceptions where a full include is required in `.h` are:
  - The class being inherited from (e.g. `#include "Blueprint/UserWidget.h"`)
  - Structs or enums used **by value** in a function signature or as a UPROPERTY (e.g. `FPanelLayoutData`, `EDiceType`)
  - Enums used in a `UFUNCTION` signature (e.g. `ETextCommit::Type`) since enums cannot be forward declared

**Example — correct:**
```cpp
// MyWidget.h
class UButton;
class UTextBlock;
```
```cpp
// MyWidget.cpp
#include "Components/Button.h"
#include "Components/TextBlock.h"
```

---

## 3. Naming Conventions

### Prefixes (UE standard)
| Prefix | Type |
|---|---|
| `U` | UObject-derived class or widget |
| `A` | AActor-derived class |
| `F` | Struct |
| `E` | Enum |
| `I` | Interface |

### Member naming
- `bCamelCase` for all booleans (e.g. `bIsExpanded`, `bChatFocused`, `bRollInProgress`)
- `Ref` suffix for cached runtime object pointers (e.g. `ChatBoxRef`, `PlayerControllerRef`)
- `On` prefix for all event handler functions (e.g. `OnLaunchCampaignButtonClicked`, `OnBackClicked`)
- `F` prefix + descriptive name for delegate types (e.g. `FOnCampaignSelected`, `FOnGameTypeSelected`)

---

## 4. Class Layout and Access

Within a UCLASS, members are grouped using `#pragma region` / `#pragma endregion`. Fields come before functions. Within each group, order is private → protected → public.

**Fields:**
1. **Widget References** — `BindWidget` UPROPERTYs (private)
2. **State** — Runtime state variables (private)
3. **Runtime References** — Cached pointers set at runtime (private)
4. **Config** — `EditAnywhere` UPROPERTYs set in Blueprint defaults (public)
5. **Components** — Subobject pointers created in the constructor (public)
6. **Events** — Delegates (`BlueprintAssignable`, public)

**Functions:**
7. **Private Methods** — Internal helpers (private)
8. **Event Handlers** — `UFUNCTION()` AddDynamic targets (private)
9. **Public Methods** — Setters, public API (public)

`#pragma region` / `#pragma endregion` are used in `.h` files only, not in `.cpp` files.

Fields precede functions in the file. Within each group, access specifiers follow **private → protected → public**. Never collapse `protected:` to `private:`; `protected` is intentional and grants subclass access. Lifecycle overrides (`NativeConstruct`, `BeginPlay`, `OnPossess`, etc.) belong under `protected:` in the functions group.

---

## 5. UPROPERTY Specifiers

| Use case | Specifier |
|---|---|
| Config values set in Blueprint defaults | `EditAnywhere, BlueprintReadWrite` |
| Read-only state visible in editor | `VisibleAnywhere, BlueprintReadOnly` |
| Per-instance state visible in editor | `VisibleInstanceOnly, BlueprintReadOnly` |
| Internal runtime refs (no editor access needed) | `UPROPERTY()` |
| Widget bindings | `UPROPERTY(meta = (BindWidget))` |
| Delegates | `UPROPERTY(BlueprintAssignable)` |

All object pointer UPROPERTYs use `TObjectPtr<T>`.

Do not add `meta = (ToolTip = "...")` when a `/** */` doc comment already documents the property — these duplicate each other.

---

## 6. UFUNCTION Rules

- Any function passed to `AddDynamic` must be marked `UFUNCTION()`.
- `AddDynamic` target functions must have a signature that exactly matches the delegate's parameter list.
  - If the delegate is zero-param, the handler must be zero-param. Store any data needed for the broadcast as private member variables and read them in the handler.
- `BlueprintCallable` is only added when Blueprint access is genuinely required.

---

## 7. Save Game Classes

All `USaveGame` subclasses must declare a slot name constant:

```cpp
// MyDataSave.h
static const FString SaveSlotName;

// MyDataSave.cpp
const FString UMyDataSave::SaveSlotName = TEXT("MyData");
```

All `SaveGameToSlot`, `LoadGameFromSlot`, and `DoesSaveGameExist` calls must reference this constant — never a hardcoded string literal.

---

## 8. Delegate Design

- Delegates carry only the data the parent needs to act — no raw widget pointers.
- Prefer passing IDs (`FGuid`) and keys (`FString`) rather than full structs when possible.
- Delegate type names are prefixed `F` and describe the event (e.g. `FOnCampaignSelected`, `FOnPanelStateChanged`).

---

## 9. String Literals

All string literals passed to UE APIs must use the `TEXT()` macro:

```cpp
// Correct
return TEXT("D20");

// Wrong
return "D20";
```

---

## 10. Documentation

### Header files (`.h`)
Every declaration gets a `/** */` doc comment on the line immediately above it:
- Class and struct declarations (on the line before `UCLASS()` / `USTRUCT()`)
- Every `UPROPERTY`
- Every public and protected method
- Private `UFUNCTION()` handlers

**Example:**
```cpp
/** Fired when the button is clicked, passing the game type name. */
UPROPERTY(BlueprintAssignable)
FOnGameTypeSelected OnGameTypeSelected;
```

### Source files (`.cpp`)
Every function definition gets a single-line `//` summary immediately before the opening brace:

```cpp
// Sets the game type label text.
void UGameTypeButton::SetLabel(const FString& Label)
{
    GameTypeLabel->SetText(FText::FromString(Label));
}
```

Comments must be written for every function, including `NativeConstruct` overrides and private handlers. Do not leave UE-generated boilerplate comments (e.g. `// Called every frame`) in place of a real summary.

---

## 11. Null Safety

### Use `nullptr`, never `NULL`
```cpp
// Correct
if (MyPtr == nullptr) { }

// Wrong
if (MyPtr == NULL) { }
```

### Use `IsValid()` for UObject pointers
A UObject pointer can be non-null but garbage collected (pending kill). A raw null check does not catch this.

```cpp
// Correct — catches both null and pending-kill
if (IsValid(ChatBoxRef)) { }

// Wrong — misses pending-kill objects
if (ChatBoxRef) { }
```

The exception is `BindWidget` references — these are guaranteed valid if the widget is properly bound and the Blueprint is set up correctly. A simple `if (Ptr)` guard is acceptable there.

---

## 12. Const Correctness

- Methods that do not modify object state must be marked `const`:
```cpp
FString GetPanelID() const;
```
- Parameters passed by reference that are not modified must be `const&`:
```cpp
void SetLabel(const FString& Label);
void ApplyPanelLayoutData(const FPanelLayoutData& LayoutData);
```
- Avoid passing large types (structs, strings, arrays) by value when `const&` is an option.

---

## 13. Super:: in Overrides

Every overridden lifecycle function must call its `Super::` version as the **first line** of the body:

```cpp
void UMyWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // ... rest of logic
}
```

This applies to: `NativeConstruct`, `BeginPlay`, `OnPossess`, `PostEditChangeProperty`, and any other virtual lifecycle override.

---

## 14. Memory Management

Never use raw `new` or `delete` for UObject-derived types. Always use UE's managed allocation:

| Situation | Use |
|---|---|
| Creating a widget | `CreateWidget<T>()` |
| Creating a UObject at runtime | `NewObject<T>()` |
| Creating a component in a constructor | `CreateDefaultSubobject<T>()` |
| Spawning an actor | `GetWorld()->SpawnActor<T>()` |

Raw `new` / `delete` is only acceptable for non-UObject types (plain C++ structs, STL types) where UE memory management does not apply.

---

## 15. Boilerplate Cleanup

UE creates stub implementations for `BeginPlay`, `Tick`, and `SetupPlayerInputComponent` in new actor/pawn classes. If these functions contain no logic beyond `Super::`, **remove them entirely** from both `.h` and `.cpp` rather than leaving empty stubs.

---

## 16. Defensive Programming

### Null checks on external pointers
Any pointer retrieved from an external source must be null-checked before use. External sources include:
- `GetOwner()`, `GetWorld()`, `GetOwningPlayer()`
- `Cast<T>()`
- `GetWidgetFromName()` / `GetTypedWidgetFromName<T>()`
- `LoadGameFromSlot()`, `CreateSaveGameObject()`
- `GetActorOfClass()`, `SpawnActor()`
- `GetGameState()`, `PlayerState`

If the check fails and execution cannot continue, log a warning and return early. No silent failures.

```cpp
APlayerController* PC = Cast<APlayerController>(GetOwner());
if (!IsValid(PC))
{
    UE_LOG(LogTemp, Warning, TEXT("UMyComponent::BeginPlay — Owner is not a PlayerController"));
    return;
}
```

### Numerical bounds
Any value with a defined valid range must be clamped at the point it is applied:

```cpp
SpringArm->TargetArmLength = FMath::Clamp(NewLength, MinZoomLength, MaxZoomLength);
```

Do not rely on validation happening somewhere else in the call chain. Clamp at the assignment site.

### Early returns over deep nesting
Functions with preconditions should validate at the top and return early rather than nesting all logic inside if-blocks:

```cpp
// Correct
if (!IsValid(ChatBoxRef))
{
    UE_LOG(LogTemp, Warning, TEXT("UChatBox::FocusChat — ChatBoxRef is null"));
    return;
}
ChatBoxRef->FocusChat();

// Avoid
if (ChatBoxRef)
{
    if (SomeOtherCondition)
    {
        ChatBoxRef->FocusChat();
    }
}
```

---

## 17. Logging

### Log levels
| Situation | Level |
|---|---|
| Expected informational event (save loaded, widget found) | `Display` |
| Recoverable issue (null ref, failed cast, missing optional asset) | `Warning` |
| Unrecoverable issue that will break functionality | `Error` |

### Log format
Every log message must include the class name and function name for traceability:

```cpp
UE_LOG(LogTemp, Warning, TEXT("UGameplayHUDComponent::BeginPlay — DiceSpawnVolume not found in level"));
UE_LOG(LogTemp, Error, TEXT("UCampaignManagerScreen::LoadCampaigns — Failed to load save game"));
UE_LOG(LogTemp, Display, TEXT("UGameplayHUDComponent::BeginPlay — Panel layout loaded successfully"));
```

Format: `ClassName::FunctionName — description of what happened`

### No silent failures
If a null check or cast fails and causes a function to return early, a log entry is mandatory. Never return silently from a failure path.

---

## 18. Include Order

In `.cpp` files, includes must follow this order with a blank line between each group:

1. The file's own header (first, always)
2. Engine and plugin headers (`"Blueprint/UserWidget.h"`, `"Components/Button.h"`, etc.)
3. Project headers (`"ChatBox.h"`, `"GameplayHUDComponent.h"`, etc.)

```cpp
// MyWidget.cpp
#include "MyWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "ChatBox.h"
#include "GameplayHUDComponent.h"
```

This order ensures that `MyWidget.h` is self-contained (missing includes surface as errors here rather than silently resolved by a prior include) and makes the dependency boundary between engine and project code visually clear.

---

## 19. Function Order in `.cpp`

Function definitions in `.cpp` must appear in the **same order** as their declarations in the `.h`. If the `.h` declaration order changes, the `.cpp` must be reordered to match.

---

## 20. Garbage Collection Safety

Every `UObject`-derived pointer stored as a class member **must** be covered by `UPROPERTY()`. A raw pointer with no `UPROPERTY` is invisible to Unreal's garbage collector — the object it points to can be collected while the pointer still holds an address, producing a dangling pointer that `IsValid()` will not reliably catch.

```cpp
// Correct — GC-tracked
UPROPERTY()
TObjectPtr<UChatBox> ChatBoxRef;

// Wrong — not GC-tracked, can dangle silently
UChatBox* ChatBoxRef;
```

This applies to all member pointers regardless of access level or whether specifiers like `EditAnywhere` are needed. Use bare `UPROPERTY()` (no specifiers) for internal runtime references that don't need editor exposure.

Local variables inside function bodies are not subject to this rule.

---

## 20. No Debug Output in Committed Code

The following are only acceptable during local debugging and must be removed before committing:

- `GEngine->AddOnScreenDebugMessage(...)`
- `UKismetSystemLibrary::PrintString(...)`
- `DrawDebugLine`, `DrawDebugSphere`, and other `DrawDebug*` calls

`UE_LOG` is the only accepted logging path in committed code. If information needs to be visible during a play session, add a `Display`-level log entry instead.
