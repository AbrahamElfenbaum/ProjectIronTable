# ProjectIronTable — Technical Design Document

**Engine:** Unreal Engine 5.7 (C++)
**Project Path:** `D:\Projects\Unreal\ProjectIronTable`
**Status:** Active Development
**Primary Game System:** D&D 5e / 2024 (expandable to other TTRPGs)

---

## Overview

ProjectIronTable is a TTRPG simulator built in Unreal Engine 5.7. It supports maps, miniatures, dice rolling, character sheets, initiative tracking, and custom user-imported content. Development is structured in small, self-contained pieces starting with the dice system.

---

## Folder Structure

### C++ Source (`Source/ProjectIronTable/`)
```
Source/ProjectIronTable/
├── Chat/              — Chat widget classes (ChatBox, ChatEntry, ChatChannel, ChatTab, ChatChannelListEntry)
├── Components/        — Actor component classes (GameplayHUDComponent, MainScreenHUDComponent)
├── Dice/              — Dice actors and data assets
├── Pawns/             — Pawn classes
├── PlayerControllers/ — Player controller classes
├── PlayerList/        — Player list widget classes (PlayerList, PlayerRow)
├── SaveLoad/          — Save game classes (PanelLayoutSave, CameraSettingsSave)
├── UI/                — Non-chat widget classes (no HUD components)
└── Utility/           — Function libraries and general-purpose helpers
```

### Content Browser (`Content/`)
```
Content/
├── Blueprints/
│   ├── Core/
│   │   ├── GameModes/          — GM_Testing, GM_Gameplay, GM_MainScreen
│   │   ├── PlayerControllers/  — PC_Testing, PC_Gameplay, PC_MainScreen
│   │   └── Components/         — BP_HUDComponent, BP_HomeScreenHUDComponent
│   ├── Dice/
│   │   ├── A_BaseDiceActor     — Base dice actor Blueprint
│   │   └── DiceActors/         — Individual die Blueprints (A_D4, A_D6, etc.)
│   └── Pawns/
│       └── P_GameplayPawn      — Camera pawn Blueprint
├── Data/DataAssets/Dice/       — DA_ prefixed dice data assets
├── Input/
│   ├── Gameplay/               — IMC_Gameplay, IA_CameraMove, IA_CameraPan, IA_CameraPanReset, IA_CameraSprint, IA_CameraZoom, IA_FocusChat
│   └── Chat/                   — IMC_Chat, IA_ExitChat, IA_ScrollChat
├── UI/
│   ├── Dice/                   — WE_DiceSelector, W_DiceSelectorManager
│   ├── Chat/                   — W_ChatBox, WE_ChatChannel, WE_ChatTab, WE_ChatEntry
│   ├── PlayerList/             — W_PlayerList, WE_PlayerRow
│   ├── Screens/                — S_GameplayScreen, S_HomeScreen, S_MainScreen, S_SettingsScreen
│   ├── Settings/               — WE_SettingsSlider
│   ├── HUD/                    — W_Taskbar, WE_TaskbarButton
│   ├── Utility/                — W_DraggablePanel, WE_DragHandle, WE_ResizeHandle
│   └── Testing/                — WE_DebugDisplay
└── Physics/Materials/          — PM_Dice
```

---

## Naming Conventions

| Asset Type        | Prefix |
|-------------------|--------|
| Actor (Blueprint) | A_     |
| Pawn (Blueprint)  | P_     |
| Game Mode         | GM_    |
| Player Controller | PC_    |
| Data Asset        | DA_    |
| Data Table        | DT_    |
| Screen            | S_     |
| Widget            | W_     |
| Widget Element    | WE_    |
| Enumeration       | E_     |
| Material          | M_     |
| Material Instance | MI_    |
| Level (final)     | L_     |
| Level (dev/test)  | Dev_   |

---

## C++ Classes

---

### Dice/

#### ADiceSpawnVolume
**Type:** `AActor` | **Place in level:** yes (one instance)

Defines the spawn area for dice. `UGameplayHUDComponent` finds it at runtime and passes it to `UDiceSelectorManager`.

**Components:** `SpawnArea` (`UBoxComponent`, root — visible/resizable in viewport)

**Key Methods:**
- `GetSpawnBox()` — returns world-space `FBox`

---

#### ABaseDiceActor
**Type:** `AActor` | **Blueprint:** `A_BaseDiceActor`, with per-die child Blueprints (`A_D4`, `A_D6`, etc.)

Parent class for all dice. Handles physics, sleep detection, roll result reading, and collision sounds.

**Config (set in Blueprint):**
| Property | Type | Default | Description |
|---|---|---|---|
| `Mass` | float | — | Rigid body mass |
| `LinearDamping` / `AngularDamping` | float | — | Physics damping |
| `ImpulseRange` / `AngularImpulseRange` | float | — | Roll impulse range |
| `FailSafeTime` | float | 10s | Auto-destroy if not settled |
| `CollisionSoundSurface` | USoundBase | — | Sound when hitting non-die surface |
| `CollisionSoundDice` | USoundBase | — | Sound when hitting another die |
| `ThrottleInterval` | float | 0.1s | Min time between collision sounds |
| `ImpulseVolumeScale` | float | 1000 | Divides impulse magnitude to get volume |

**Key Behavior:**
- `BeginPlay` — calls `SetNotifyRigidBodyCollision(true)` and binds `OnComponentHit` on both meshes
- On hit — throttles by `LastHitTime`, picks sound by other actor type, scales volume by impulse
- On sleep — calls `SetSimulatePhysics(false)` on settled mesh
- `bWasKept` (`bool`, default `true`) — set `false` on losing die during advantage/disadvantage rolls

**Delegates:**
- `OnFailsafeDestroy` (`EDiceType`) — broadcast if mesh hasn't settled within `FailSafeTime`

> **Note:** `OnComponentSleep` requires **Generate Wake Events** checked on the mesh component in Blueprint.

---

#### UDiceData
**Type:** `UPrimaryDataAsset`

Stores per-die configuration (mesh, face values, die type, etc.).

---

### Chat/

#### UChatBox
**Type:** `UUserWidget` | **Blueprint:** `W_ChatBox`

Tabbed chat container. Manages channels, routing, and input.

**Bound Widgets:**
| Name | Type |
|---|---|
| `TabBar` | `UHorizontalBox` |
| `ClosedChannelContainer` | `UVerticalBox` |
| `ChannelContainer` | `UWidgetSwitcher` |
| `EditableText` | `UEditableText` |
| `ChannelListButton` | `UButton` |

**Config (EditAnywhere):** `ChannelClass`, `TabClass`, `ChatEntryClass`, `ChannelListEntryClass`

**Blueprint layout:** `Border → SizeBox → VerticalBox → [HBox(TabBar, ChannelListButton)], ClosedChannelContainer, ChannelContainer, EditableText`

**Key Methods:**
- `CreateChannel(TArray<FString> Participants)` — creates channel + tab, wires delegates, returns channel. Empty participants = Server tab (not closeable).
- `SwitchToChannel(UChatChannel*)` — activates channel, clears notification and input
- `AddChatMessage(Message, Participants, bIsSender)` — routes to correct channel (creates if needed); auto-reopens closed channels on incoming message; shows notification if not active
- `FocusChat()` / `ExitChat()` — manage input mode. `ExitChat` does **not** clear the input field.
- `AppendToInput(FString)` — appends text to current input value
- `GetActiveChannelParticipants()` — returns active channel's participants, or `{}` for Server
- `TrySendPrivateRollMessage()` — if `@Name` tokens in input, sends them as a message and clears input; noop otherwise

> **Note:** Channel matching checks `Participants.Num()` equality before content — required to avoid partial matches.
> **Note:** `SwitchToChannel` must be `UFUNCTION()` for `AddDynamic` to work.
> **Note:** `ExitChat` must not clear the input — players type `@Names` before clicking Roll.

---

#### UChatChannel
**Type:** `UUserWidget` | **Blueprint:** `WE_ChatChanel`

Represents one channel/tab's message list.

**Bound Widgets:** `ScrollBox` (`UScrollBox`)

**Config:** `ChatEntryClass`, `DisplayName` (client-local label — safe to rename; routing uses `Participants`), `Participants` (`TArray<FString>`), `ScrollMultiplier` (default 60)

**Key Methods:**
- `AddChatMessage(FString)` — creates and appends a `UChatEntry`
- `Scroll(bool bUp)` — adjusts scroll offset by `ScrollMultiplier`

---

#### UChatTab
**Type:** `UUserWidget` | **Blueprint:** `WE_ChatTab`

Clickable tab button in the tab bar.

**Bound Widgets:** `TabButton`, `CloseButton` (`UButton`), `TabLabel` (`UTextBlock`), `NotificationIndicator` (`UWidget`)

**Key Methods:**
- `SetInteractable(bool)` — enables/disables `TabButton` (active tab is disabled so it can't be re-clicked)
- `SetCloseable(bool)` — shows/hides `CloseButton` (Server tab passes `false`)
- `ShowNotification()` / `ClearNotification()`

**Delegates:** `OnTabClicked` (→`UChatChannel*`), `OnTabClosed` (→`UChatChannel*`)

---

#### UChatEntry
**Type:** `UUserWidget` | **Blueprint:** `WE_ChatEntry`

Single message row. **Bound Widget:** `TextBlock`. **Expose on Spawn:** `Message`.

---

#### UChatChannelListEntry
**Type:** `UUserWidget` | **Blueprint:** `WE_ChatChannelListEntry`

Row in the closed channels dropdown. **Bound Widgets:** `EntryButton`, `EntryLabel`.

**Delegate:** `OnEntryClicked` (→`UChatChannel*`)

---

### PlayerList/

#### UPlayerRow
**Type:** `UUserWidget` | **Blueprint:** `WE_PlayerRow`

Single row in the player list. Clicking the address button @-mentions that player.

**Bound Widgets:** `NameLabel` (`UTextBlock`), `AddressButton` (`UButton`)

**Key Methods:** `SetPlayerName(FString)`

**Delegate:** `OnAddressClicked` (→`FString PlayerName`)

---

#### UPlayerList
**Type:** `UUserWidget` | **Blueprint:** `W_PlayerList`

Collapsible scrollable list of connected players.

**Bound Widgets:** `ScrollBox` (`UScrollBox`), `ToggleButton` (`UButton`)

**Config:** `PlayerRowClass` (`TSubclassOf<UPlayerRow>`)

**Key Methods:**
- `PopulateList()` — clears and rebuilds rows from `GameState->PlayerArray`

**Delegate:** `OnAddressClicked` (bubbled up from each row)

> **Note:** `PopulateList` must also be called on toggle open — `GameState->PlayerArray` may be empty at construct time.

---

### SaveLoad/

#### FPanelLayoutData
**Type:** `USTRUCT`

Stores saved layout for one panel. Fields: `Position` (`FVector2D`), `Size` (`FVector2D`), `bVisible` (`bool`, default `true`).

---

#### UPanelLayoutSave
**Type:** `USaveGame` | **Slot:** `"PanelLayout"`, index 0

Stores panel layout for all registered panels.

**Fields:** `PanelLayouts` (`TMap<FString, FPanelLayoutData>`) — keyed by `PanelID`

---

#### UCameraSettingsSave
**Type:** `USaveGame` | **Slot:** `"CameraSettings"`, index 0

Persists all nine camera config properties across sessions.

| Field | Default |
|---|---|
| `MinCameraMovementSpeed` | 5 |
| `MaxCameraMovementSpeed` | 20 |
| `CameraSpeedMultiplier` | 2 |
| `MinCameraPitch` | -15 |
| `MaxCameraPitch` | 45 |
| `CameraPanSpeedMultiplier` | 5 |
| `MinZoomLength` | 100 |
| `MaxZoomLength` | 2500 |
| `ZoomSpeed` | 50 |

---

### Components/

#### UMainScreenHUDComponent
**Type:** `UActorComponent` | **Blueprint:** `BP_HomeScreenHUDComponent` (rename pending)
**Owner:** `AMainScreenController`

Manages the main screen UI — creates the widget, caches all refs, and handles screen navigation and settings.

**Config:**
- `MainScreenClass` (`TSubclassOf<UUserWidget>`) — **must be set on the component instance inside `PC_MainScreen`**, not on a standalone component Blueprint

**Widget names (must match exactly):**
- Home screen: `PlayButton`, `JoinButton`, `LibraryButton`, `SettingsButton`, `QuitButton`
- Settings screen: `MinCamSpeed`, `MaxCamSpeed`, `CamSpeedMultiplier`, `MinPitch`, `MaxPitch`, `PanMultiplier`, `MinZoom`, `MaxZoom`, `ZoomSpeed`, `ApplyButton`, `ResetButton`, `BackButton`
- Root: `ScreenSwitcher` (`UWidgetSwitcher`), `HomeScreen`, `SettingsScreen`

**Handlers:**
- `OnPlayClicked` → `OpenLevel("L_Gameplay")`
- `OnQuitClicked` → `QuitGame`
- `OnSettingsClicked` → `ScreenSwitcherRef->SetActiveWidgetIndex(1)`
- `OnBackClicked` → `ScreenSwitcherRef->SetActiveWidgetIndex(0)` — shared handler, not region-specific; reused by all future screens with a Back button
- `OnResetClicked` → iterates `SettingsSliders` TArray, calls `ResetToDefault()` on each, then calls `OnApplyClicked()`
- `OnApplyClicked` → creates `UCameraSettingsSave`, reads all 9 slider values into fields, calls `SaveGameToSlot("CameraSettings", 0)`
- `OnJoinClicked`, `OnLibraryClicked` → stubs (not yet implemented)

**Runtime:** `SettingsSliders` (`TArray<USettingsSlider*>`, `UPROPERTY()`) — all 9 slider refs in order for batch reset operations.

**BeginPlay save init:** After caching slider refs, checks `DoesSaveGameExist("CameraSettings", 0)`. If yes — loads and casts to `UCameraSettingsSave`, calls `SetValue` on each slider. If no — calls `OnResetClicked()` to apply defaults and create the save file on first launch.

---

#### UGameplayHUDComponent
**Type:** `UActorComponent` | **Replicated:** yes
**Owner:** `AGameplayController`

Manages the gameplay HUD lifecycle and all chat networking.

**Config:**
- `GameplayScreenClass` (`TSubclassOf<UUserWidget>`)

**Widget names it searches for (must match):** `DiceSelectorManager`, `ChatBox`, `PlayerList`, `Taskbar`, `DicePanel`, `ChatPanel`, `PlayersPanel`

**Key Methods:**
- `FindAndRegisterPanel(WidgetName, Label)` — finds `UDraggablePanel`, registers with taskbar, assigns ID, binds layout save delegates
- `SavePanelLayout()` — writes all panel positions/sizes to `"PanelLayout"` save slot
- `LoadPanelLayout()` — loads and applies saved panel layout on startup
- `FocusChat()` / `ExitChat()` / `ScrollChat(bool)` — delegate to `ChatBoxRef`
- `OnRollInitiated()` — calls `TrySendPrivateRollMessage` before dice spawn

**RPCs:**
- `SendChatMessageOnServer` (Server, Reliable) — resolves sender name, builds participant list, routes to each player's HUD
- `AddChatMessageOnOwningClient` (Client, Reliable) — delivers message to `ChatBoxRef`

> **Note:** Always place the Blueprint variant (`W_Taskbar`, etc.) in the screen widget — placing the raw C++ class causes null `BindWidget` crashes on first access.

---

### UI/

#### UDragHandle / UResizeHandle
**Type:** `UUserWidget` | **Blueprints:** `WE_DragHandle`, `WE_ResizeHandle`

Hit-testable widgets that capture mouse and forward drag/resize events to their parent `UDraggablePanel` via `GetTypedOuter<UDraggablePanel>()`. Guard mouse move with a `bActive` flag — `NativeOnMouseMove` fires on hover too.

---

#### UDraggablePanel
**Type:** `UUserWidget` | **Blueprint:** `W_DraggablePanel`

Makes any content widget draggable and resizable at runtime. **Must be inside a `UCanvasPanel` with top-left anchor.**

**Bound Widgets:** `DragHandle` (`UWidget`), `ContentSlot` (`UNamedSlot`), `ResizeCorner` (`UWidget`), `TitleText` (`UTextBlock`)

**Config (EditAnywhere):** `TitleBarHeight`, `MinSize` (`FVector2D`), `PanelTitle` (`FText`), `DefaultPosition` (`FVector2D`), `DefaultSize` (`FVector2D`)

**Key Methods:**
- `SetPanelID(FString)` / `GetPanelID()` — ID used as save key
- `GetPanelLayoutData()` — returns current position, size, visibility as `FPanelLayoutData`
- `ApplyPanelLayoutData(const FPanelLayoutData&)` — restores position, size, visibility
- `ResetToDefaultLayout()` — sets position and size to `DefaultPosition`/`DefaultSize`, broadcasts `OnPanelStateChanged` to trigger save

**Delegate:** `OnPanelStateChanged` (zero-param) — broadcast on drag/resize/reset stop; triggers `SavePanelLayout`

> **Note:** `DefaultPosition` and `DefaultSize` must be set as `EditAnywhere` properties in the Blueprint — canvas slot values are 0,0 at `NativeConstruct` time and cannot be cached there reliably.

---

#### UDiceSelector
**Type:** `UUserWidget` | **Blueprint:** `WE_DiceSelector`

UI row for selecting a die type and count. All logic is C++; Blueprint is layout only.

**Bound Widgets:** `TypeText`, `CountText` (`UTextBlock`), `IncreaseButton`, `DecreaseButton` (`UButton`)

**Config:** `DiceClass` (`TSubclassOf<ABaseDiceActor>`), `DiceType` (`EDiceType`), `DiceCount` (read-only)

---

#### UDiceSelectorManager
**Type:** `UUserWidget` | **Blueprint:** `W_DiceSelectorManager`

Manages all dice selectors and initiates rolls.

**Bound Widgets:** `D4`–`D100` (`UDiceSelector`), `NormalRollButton`, `AdvantageRollButton`, `DisadvantageRollButton`, `RollButton` (`UButton`)

**Config:** `SpawnVolume` (set at runtime by `UGameplayHUDComponent`), `Impulse`, `AngularImpulse`, `ImpulseRange`, `AngularImpulseRange`, `TimeBeforeDestroyingDice` (5s)

**Roll Modes:** Normal / Advantage / Disadvantage — mode buttons enabled only when exactly one selector has `DiceCount == 1`. Advantage/Disadvantage spawn 2 dice and keep only the higher/lower result; losing die gets `bWasKept = false`.

**Delegates:**
- `OnAllDiceRolled` (`TArray<FRollResult>`, `EDiceRollMode`)
- `OnDiceFailsafeDestroyed` (`EDiceType`)
- `OnRollInitiated` (zero-param) — broadcast at the **very start** of `RollDice`, before spawning

> **Note:** `bRollInProgress` must be set `true` before the spawning loop, not after — otherwise `ResetCount` resets `RollMode` mid-roll.
> **Note:** `ADiceSpawnVolume` must be placed in the level or `RollDice` will log a warning and do nothing.

---

#### UTaskbar / UTaskbarButton
**Type:** `UUserWidget` | **Blueprints:** `W_Taskbar`, `WE_TaskbarButton`

Taskbar at the bottom of the screen. Each button toggles a tracked `UUserWidget` between visible and collapsed.

**UTaskbar — Bound Widgets:** `ButtonContainer` (`UHorizontalBox`), `ResetButton` (`UButton`)

**UTaskbar — Key Methods:**
- `RegisterWidget(UUserWidget*, FString Label)` → `UTaskbarButton*` — creates button, adds to container, returns it for delegate binding
- `ResetLayout()` — iterates `ButtonContainer` children, casts each to `UTaskbarButton`, casts `TrackedWidget` to `UDraggablePanel`, calls `ResetToDefaultLayout()` on each. Bound to `ResetButton` in `NativeConstruct`.

**UTaskbarButton — Delegate:** `OnToggled` (zero-param) — broadcast after each toggle

> **Note:** Toggle logic must use `if (Collapsed) → Visible; else → Collapsed` — default widget visibility is `SelfHitTestInvisible`, not `Visible`.
> **Note:** `ResetLayout` casts `TrackedWidget` to `UDraggablePanel` — non-panel tracked widgets are silently skipped via null check.

---

#### USettingsSlider
**Type:** `UUserWidget` | **Blueprint:** `WE_SettingsSlider`

Labeled slider paired with an editable text field. Supports optional pairing with another slider to enforce a min/max gap.

**Bound Widgets:** `TitleText` (`UTextBlock`), `ValueSlider` (`USlider`), `ValueText` (`UEditableText`)

**Config (EditAnywhere):**
| Property | Type | Default | Description |
|---|---|---|---|
| `Title` | FText | — | Label text |
| `SliderMin` / `SliderMax` | float | 0 / 1 | Slider range |
| `DefaultValue` | float | 0 | Value on reset |
| `PairedSlider` | USettingsSlider* | null | Other slider in a min/max pair |
| `PairGap` | float | 0.1 | Minimum gap between paired values |
| `bIsMin` | bool | false | If true, this slider is capped below the paired slider |

**Key Methods:**
- `GetValue()` — returns current slider value
- `SetValue(float)` — sets slider + text directly, no pair clamping (used for load/reset)
- `ResetToDefault()` — calls `SetValue(DefaultValue)`

> **Note:** `OnSliderValueChanged` only calls `SetValue` when the clamped value differs — calling it unconditionally causes infinite recursion.

---

### Pawns/

#### AGameplayPawn
**Type:** `APawn` | **Blueprint:** `P_GameplayPawn`

Top-down camera rig.

**Component hierarchy:** `Root (USceneComponent) → Sphere (UStaticMeshComponent) → SpringArm → Camera`

All components are public so `AGameplayController` can access `SpringArm->TargetArmLength`.

---

### PlayerControllers/

#### AMainScreenController
**Type:** `APlayerController` | **Blueprint:** `PC_MainScreen`

UI-only controller for the main screen. No camera pawn, no Enhanced Input.

**Constructor:** `CreateDefaultSubobject<UMainScreenHUDComponent>("HUDComponent")`

**BeginPlay:** `bShowMouseCursor = true`, `FInputModeUIOnly()`

---

#### AGameplayController
**Type:** `APlayerController` | **Blueprint:** `PC_Gameplay`

Central hub for all player input and HUD management.

**Constructor:** `CreateDefaultSubobject<UGameplayHUDComponent>("HUDComponent")`

**Input (bound in `OnPossess`):**

| Action | Type | Behavior |
|---|---|---|
| `IA_CameraMove` | Vector2D | Move along pawn forward/right; `Delta.Z` zeroed for flat movement |
| `IA_CameraPan` | bool | Hold to rotate pawn: mouse X → Yaw, mouse Y → Pitch (clamped) |
| `IA_CameraPanReset` | — | Reset pitch to -15, preserve yaw |
| `IA_CameraSprint` | bool | Hold: apply `CameraSpeedMultiplier`; release: restore 1x |
| `IA_CameraZoom` | float | Adjust `SpringArm->TargetArmLength` by `Sign * ZoomSpeed` (clamped) |
| `IA_FocusChat` | — | Swap in `IMC_Chat` (priority 1), delegate to HUD |
| `IA_ExitChat` | — | Swap out `IMC_Chat`, delegate to HUD |
| `IA_ScrollChat` | float | Delegate sign to `HUDComponent->ScrollChat()` |

**Camera Properties (defaults):**

| Property | Default |
|---|---|
| `MinCameraMovementSpeed` | 5 |
| `MaxCameraMovementSpeed` | 20 |
| `CameraSpeedMultiplier` | 2 |
| `MinCameraPitch` | -15 |
| `MaxCameraPitch` | 45 |
| `CameraPanSpeedMultiplier` | 5 |
| `MinZoomLength` | 100 |
| `MaxZoomLength` | 2500 |
| `ZoomSpeed` | 50 |

**Settings Methods:**
- `ValidateCameraSettings()` — clamps all 9 properties to valid ranges; called by `PostEditChangeProperty` and `ApplyCameraSettings`
- `ApplyCameraSettings(const UCameraSettingsSave*)` — copies fields from save object, calls Validate
- `SaveCameraSettings()` — writes all 9 fields to `"CameraSettings"` slot
- `BeginPlay` — loads and applies `"CameraSettings"` if it exists

> **Note:** `bCanCameraMove` is a plain `bool` (no `UPROPERTY`) to avoid Blueprint CDO override.
> **Note:** `PostEditChangeProperty` is wrapped in `#if WITH_EDITOR` — for runtime validation use `ValidateCameraSettings()` directly.

---

### Utility/

#### UFunctionLibrary
**Type:** `UBlueprintFunctionLibrary`

General-purpose helper functions accessible from C++ and Blueprint.

**Functions:**
- `GetDiceName(EDiceType)` → `FString` — returns display name (e.g. `"D20"`) *(BlueprintPure)*
- `GetTypedWidgetFromName<T>(UUserWidget* Widget, FName Name)` → `T*` — template; casts result of `GetWidgetFromName`. Logs a warning if `Widget` is null or the cast fails. **C++-only** (no `UFUNCTION` — templates can't be `UFUNCTION`). Use this everywhere instead of `Cast<T>(Widget->GetWidgetFromName(...))`.

> **Note:** The template definition must live entirely in the `.h` — no `.cpp` entry needed. Define it inside the class body (implicit `inline`) or outside with `inline`.

---

## Build Workflow

**Every session:**
1. Build in Visual Studio (Ctrl+Shift+B) — do this **before** opening the `.uproject`
2. Open the editor after the build succeeds

**When things get weird** (warnings persist, crashes, odd behavior, or a renamed class won't appear):
1. Close the editor
2. Delete `Intermediate/` and `Binaries/` from the project root
3. Right-click `.uproject` → Generate Visual Studio project files
4. Build in Visual Studio
5. Open the editor

> Never let Unreal compile C++ on its own — Blueprint widgets that inherit from C++ classes will show "invalid parent class" warnings if the editor loads before C++ classes are registered.

---

## Build Configuration Notes

- All source subdirectories must be added to `PublicIncludePaths` in `ProjectIronTable.Build.cs`
- Uses `Path.Combine(ModuleDirectory, "FolderName")` — requires `using System.IO;` at the top
- This allows `#include "FileName.h"` with no path prefix from any folder in the module
- **Current registered folders:** `Chat`, `Components`, `Dice`, `UI`, `Utility`, `Pawns`, `PlayerControllers`, `PlayerList`, `SaveLoad`

---

## Developer Notes / Known Gotchas

### Unreal Build & Editor
- After adding new files/folders, regenerate VS project files — new folders won't appear in Content Browser otherwise
- `E1696` in VS is an IntelliSense error, not a real compiler error — build anyway to confirm
- After adding a module to `Build.cs`, regenerate VS project files so IntelliSense picks up the new paths
- `UPROPERTY` category names with spaces must use quoted strings: `Category = "My Category"` — unquoted causes UHT001
- Unreal's C++ class wizard generates incorrect include paths for files in subfolders — always verify after creation
- **Renaming a C++ class** requires a clean `Intermediate/` + `Binaries/` wipe or the new class may not appear in the editor. Blueprints parented to the old class will be orphaned and must be reparented manually.

### Blueprint & UObject
- **Blueprint CDO overrides C++ constructor defaults** — any `UPROPERTY` on a Blueprint subclass uses its saved CDO value. If a Blueprint was created with an old default, it keeps that value. Fix: set the value in the Blueprint details panel.
- **`CreateDefaultSubobject` component properties must be set on the component instance inside the owning Blueprint** — values set on a standalone `BP_MyComponent` Blueprint are never read. Open `PC_MainScreen`, select the component, set the property there.
- `NewObject` cannot create components in constructors — use `CreateDefaultSubobject` (handles registration automatically)
- **Always place Blueprint widget variants in the screen**, not raw C++ classes — `GetWidgetFromName` on a raw C++ widget finds it but its `BindWidget` members are null, causing a crash

### Input
- `FInputModeUIOnly()` does **not** block Enhanced Input actions — swap the IMC in/out via `GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()` to truly block game input
- For `APlayerController`, Enhanced Input setup goes in `OnPossess`, not `BeginPlay`
- `Cast<UEnhancedInputComponent>(InputComponent)` silently returns null if the Default Input Component Class in Project Settings is not set to `EnhancedInputComponent`
- `ETextCommit::OnCleared` fires when Escape is pressed in a `UEditableText` — handle exit-on-Escape in `OnTextCommitted`, not via input action
- `ETextCommit::OnUserMovedFocus` fires when the user clicks away — use this for click-outside-to-exit
- When `UEditableText` commits on Enter, it also fires `OnUserMovedFocus` immediately — call `FocusChat()` at the end of the Enter handler to stay in chat

### Widgets & UI
- `BindWidget` members are null at member declaration time — always do setup work in `NativeConstruct`, not the header
- **`BindWidget` members must use `TObjectPtr<Type>`, not `TSubclassOf<Type>`** — `TSubclassOf` holds a class reference; the binding produces null silently
- `AddDynamic` targets must be `UFUNCTION()` — without it the binding silently fails
- `NativeOnMouseButtonDown` on a parent `UUserWidget` does not fire for child widget clicks — use separate `UDragHandle`/`UResizeHandle` subclasses that capture the mouse and call back into the parent
- `NativeOnMouseMove` fires on hover, not just drag — guard with a `bActive` flag
- **`UDraggablePanel` must be inside a `UCanvasPanel` with top-left anchor** — other containers return null from `Cast<UCanvasPanelSlot>` and drag/resize silently does nothing; non-top-left anchors cause opposite edges to shift on resize
- `UButton` content padding is in the Style asset, not the slot — expand Style → Content Padding in the Details panel to change it
- **Taskbar toggle:** use `if (Collapsed) → Visible; else → Collapsed` — default widget visibility is `SelfHitTestInvisible`, not `Visible`; strict equality check silently fails
- `UPlayerList::PopulateList` must be called on toggle open — `GameState->PlayerArray` may be empty at construct time
- `UUserWidget` properties cannot be set by dragging level actors in the editor — assign at runtime from a component that has access to both

### Physics & Dice
- `OnComponentHit` will not fire unless `SetNotifyRigidBodyCollision(true)` is called on the mesh — binding the delegate alone is not enough
- `OnComponentSleep` will not fire unless **Generate Wake Events** is checked on the mesh component in Blueprint
- `ADiceSpawnVolume` must be placed in the level — `RollDice` logs a warning and does nothing if it's missing
- **`bRollInProgress` must be set `true` before calling `ResetCount`** — `ResetCount` triggers `UpdateAdvantageButtonState`, which resets `RollMode` to Normal if `bRollInProgress` is still false

### Networking & RPCs
- **`Participants` passed to `AddChatMessageOnOwningClient` must include the sender** — add `SenderName` with `AddUnique` before passing to clients; without it, channels on recipients' clients are missing the sender
- Channel matching must check `Participants.Num()` equality before content — a subset-participant channel can otherwise incorrectly match

### C++ Patterns
- Declaring `Type* MemberName = Cast<...>` in `BeginPlay` creates a local variable that shadows the member — use `MemberName = Cast<...>` (no type prefix)
- `FRotator(Pitch, Yaw, Roll)` — constructor order is **not** (X, Y, Z); wrong order causes violent camera bouncing
- `GetActorForwardVector()` has a Z component when pitched — zero out `Delta.Z` after computing movement to keep it flat
- `FMath::Sign(float)` returns 1.f / -1.f / 0.f — good for collapsing scroll direction checks
- `UEnum::GetValueAsString()` returns `"EnumClass::ValueName"` — strip the prefix with `RightChop(str.Find("::") + 2)`
- `UPhysicalMaterial` requires the `PhysicsCore` module in `Build.cs`
- A `USTRUCT` or `UCLASS` member that holds raw `UObject*` pointers (including `TArray<UObject*>`) must use `UPROPERTY()` — without it the GC can't track the references and they may be prematurely collected
- `USlider::SetValue` fires `OnValueChanged` the same as a user drag — guard with `ClampedValue != Value` to prevent recursion
- **Canvas slot position/size are 0,0 at `NativeConstruct` time** — do not cache `CanvasSlot->GetPosition()/GetSize()` there for use as defaults. Use `EditAnywhere UPROPERTY` fields instead and set them manually in the Blueprint.
- **C++ template functions cannot be `UFUNCTION()`** — templated methods are not compatible with Unreal's reflection system. Blueprint-accessible utilities need a non-template wrapper with a `UClass*` + manual cast, or just remain C++-only

---

## Roadmap

> Phases are not strictly time-boxed — each phase should be stable and functional before moving to the next.

---

### Phase 1 — Dice System (In Progress)

- [x] Base dice actor (`ABaseDiceActor`)
- [x] Dice data asset (`UDiceData`)
- [x] Dice selector widget (`UDiceSelector`)
- [x] Dice selector manager (`UDiceSelectorManager`)
- [x] Individual die blueprints (D4, D6, D8, D10, D12, D20)
- [x] Dice meshes and materials (*Dungeons of Dice* by NNJohn)
- [x] Physics-based roll simulation
- [x] Roll result reading (face detection via normal dot product)
- [x] Roll result display in chat
- [x] Re-roll fix — cancels destroy timer and clears old dice at start of `RollDice`
- [x] Advantage / disadvantage — any die type, not just D20
- [x] Private / hidden rolls — `@Name` tokens in chat input before rolling
- [x] Collision sound effects — surface vs. die sounds, volume scaled by impulse, throttled
- [ ] Visual effects for dice rolls
- [ ] Custom dice support (user-importable meshes and face values)
- [ ] Physics tuning (mass, damping, impulse values)
- [ ] Replace asset pack with original dice models

---

### Phase 2 — Core Game Setup (In Progress)

- [x] Test and production game modes / player controllers
- [x] `AGameplayController` — all input, HUD component, camera movement
- [x] `AGameplayPawn` — top-down camera rig
- [x] Camera movement — pan, zoom, sprint, pan reset, editor property validation
- [x] `AMainScreenController` + `UMainScreenHUDComponent` — main screen flow
- [x] `S_MainScreen` — `WidgetSwitcher` root; `S_HomeScreen` index 0, `S_SettingsScreen` index 1
- [ ] **Runtime camera settings menu** (`USaveGame`-based)
  - [x] `UCameraSettingsSave` — save class with all 9 camera fields
  - [x] `AGameplayController` save/load/validate — `ValidateCameraSettings`, `ApplyCameraSettings`, `SaveCameraSettings`; loads on `BeginPlay`
  - [x] `USettingsSlider` — reusable slider+text widget with paired min/max clamping
  - [x] `WE_SettingsSlider` — Blueprint widget layout
  - [x] `S_SettingsScreen` — 9 sliders wired and grouped; Apply, Reset, Back buttons added
  - [x] `UMainScreenHUDComponent` settings wiring — all handlers wired; save/load on BeginPlay; first-launch defaults handled
- [x] Private messaging — `@Name` syntax, per-conversation tabs, server-side routing
- [x] Panel layout persistence — `UPanelLayoutSave`; saved on drag/resize/toggle; restored on startup
- [x] Taskbar minimize system — `UTaskbar` + `UTaskbarButton`
- [x] Draggable and resizable panels — `UDraggablePanel`, `UDragHandle`, `UResizeHandle`
- [x] Close and reopen private chat tabs
- [ ] Session management (start, load, save)
- [ ] GM permissions system
- [ ] Session player cap (default 8, removable)
- [ ] Tab renaming (client-local)
- [ ] Chat log persistence
- [ ] Shared notes

---

### Phase 3 — Maps

- [ ] Flat image map support
- [ ] Tile-based map system
- [ ] Grid overlay (square and hex)
- [ ] User-importable maps and tile sets
- [ ] Fog of war
- [ ] Lighting and atmosphere controls
- [ ] Entity-based vision system

---

### Phase 4 — Miniatures

- [ ] Base miniature actor/pawn
- [ ] Grid placement and movement
- [ ] Scale system (combat vs. non-combat maps)
- [ ] Default miniature options
- [ ] Mini labels (name, HP, conditions)
- [ ] User-importable meshes (format TBD)

---

### Phase 5 — D&D 5e / 2024 Game System

- [ ] Character sheet (stats, HP, spell slots, conditions, inventory)
- [ ] Initiative tracker + GM staging list
- [ ] Initiative rolling (manual and automatic)
- [ ] Spell management
- [ ] Condition tracking (auto-apply, toggleable)
- [ ] Attack rolls, saving throws, skill checks tied to stats
- [ ] Monster/NPC stat blocks
- [ ] Basic combat flow (action economy)

---

### Phase 6 — UI & Polish

- [ ] UI theming system — decoupled from game system; per-player customization
- [ ] Panel notification system
- [ ] Replace default Unreal UI assets with custom art
- [ ] Sound effects system
- [ ] Visual effects system
- [ ] Music / ambience support
- [ ] User-uploadable music

---

### Phase 7 — Custom Content & Extensibility

- [ ] `UAssetImporter` — generic importer (extensions, destination, dialog, copy, delegate)
- [ ] Asset Library screen — file explorer UI; folder-based; import button + drag-and-drop
- [ ] Image import (PNG/JPG → Maps)
- [ ] Audio import (WAV → Sounds)
- [ ] Mesh import — deferred; requires runtime loader plugin
- [ ] Custom dice import
- [ ] Additional TTRPG systems beyond D&D 5e
- [ ] Player-to-player asset sharing
- [ ] Cloud asset storage (future)

---

### Multiplayer Architecture (Pending Research)

**Decided:**
- GM role and host role are separate
- Default player cap: 8 (GM included); removable; no hard engine limit
- When no GM is present, certain actions lock (moving NPCs, editing stats); full policy TBD

**Needs research:**
- Listen server vs. dedicated server
- Session discovery / join flow (direct IP, lobby, friend invite)

---

*Last updated: 2026-04-02* — `UMainScreenHUDComponent` settings fully complete: `OnApplyClicked` implemented, BeginPlay loads saved values and handles first-launch defaults. `UDraggablePanel` gained `DefaultPosition`/`DefaultSize` (EditAnywhere) and `ResetToDefaultLayout()`. `UTaskbar` gained `ResetButton` + `ResetLayout()` — resets all registered panels to their Blueprint-configured defaults without affecting visibility.

---

Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
