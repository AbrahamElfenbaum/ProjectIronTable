# ProjectIronTable — Technical Design Document

**Engine:** Unreal Engine 5.7 (C++)
**Project Path:** `D:\Projects\Unreal\ProjectIronTable`
**Status:** Active Development
**Primary Game System:** D&D 5e / 2024 (expandable to other TTRPGs)

---

## Overview

ProjectIronTable is a TTRPG (Tabletop Role-Playing Game) simulator built in Unreal Engine 5.7. The goal is to provide a virtual tabletop experience that supports maps, miniatures, dice rolling, character sheets, initiative tracking, and more — with support for custom user-imported content. Development is structured in small, self-contained pieces starting with the dice system.

---

## Folder Structure

### C++ Source (`Source/ProjectIronTable/`)
```
Source/ProjectIronTable/
├── Chat/              — Chat widget classes (ChatBox, ChatEntry, ChatChannel, ChatTab, ChatChannelListEntry)
├── Dice/              — Dice actors and data assets
├── Pawns/             — Pawn classes
├── PlayerControllers/ — Player controller classes
├── PlayerList/        — Player list widget classes (PlayerList, PlayerRow)
├── UI/                — Non-chat widget classes and HUD component
└── Utility/           — Function libraries and general-purpose helpers
```

### Content Browser (`Content/`)
```
Content/
├── Blueprints/
│   ├── Core/
│   │   ├── GameModes/          — GM_Testing, GM_Gameplay
│   │   ├── PlayerControllers/  — PC_Testing, PC_Gameplay
│   │   └── Components/         — BP_HUDComponent (in progress)
│   ├── Dice/
│   │   ├── A_BaseDiceActor     — Base dice actor Blueprint
│   │   └── DiceActors/         — Individual die Blueprints (A_D4, A_D6, etc.)
│   ├── Pawns/
│   │   └── GameplayPawn    — Camera pawn Blueprint (P_GameplayPawn)
│   └── Utility/
├── Data/
│   ├── DataAssets/
│   │   └── Dice/               — DA_ prefixed dice data assets
│   └── DataTables/
├── Levels/
│   └── Dev/                    — Development/test levels
├── Meshes/
│   └── Dice/                   — Dice static meshes (Dungeons of Dice by NNJohn)
├── Materials/
│   └── Dice/                   — Dice materials (Dungeons of Dice by NNJohn)
├── Textures/
├── Input/
│   ├── Gameplay/               — IMC_Gameplay, IA_CameraMove, IA_CameraPan, IA_CameraPanReset, IA_CameraSprint, IA_CameraZoom, IA_FocusChat
│   └── Chat/                   — IMC_Chat, IA_ExitChat, IA_ScrollChat
├── UI/
│   ├── Dice/                   — Dice widget elements (WE_DiceSelector, WE_DiceSelectorManager)
│   ├── Chat/                   — Chat widgets (W_ChatBox, WE_ChatChannel, WE_ChatTab, WE_ChatEntry)
│   ├── PlayerList/             — Player list widgets (W_PlayerList, WE_PlayerRow)
│   ├── Screens/                — Full screen widgets (S_GameplayScreen)
│   ├── Utility/                — Reusable utility widgets (W_DraggablePanel, WBP_DragHandle, WBP_ResizeHandle)
│   └── Testing/                — Debug/test widgets (WE_DebugDisplay)
```

---

## Naming Conventions

| Asset Type          | Prefix |
|---------------------|--------|
| Actor (Blueprint)   | A_     |
| Pawn (Blueprint)    | P_     |
| Game Mode           | GM_    |
| Player Controller   | PC_    |
| Data Asset          | DA_    |
| Data Table          | DT_    |
| Screen              | S_     |
| Widget              | W_     |
| Widget Element      | WE_    |
| Enumeration         | E_     |
| Material            | M_     |
| Material Instance   | MI_    |

---

## C++ Classes

### Dice/
- **`ADiceSpawnVolume`** — `AActor` subclass. Defines the area where dice can spawn. Has a single `UBoxComponent` (`SpawnArea`) as the root component — this makes the volume visible and resizable in the editor viewport. Exposes `GetSpawnBox()` which returns the world-space `FBox` via `SpawnArea->Bounds.GetBox()`. Place one in the level; `UGameplayHUDComponent` finds it automatically via `GetActorOfClass` and passes it to `UDiceSelectorManager` at runtime.
- **`ABaseDiceActor`** — Parent class for all dice actors. Handles mesh assignment, physics sleep detection, and roll result reporting. All individual die types inherit from this. Exposes `Mass`, `PhysicalMaterial`, `LinearDamping`, `AngularDamping`, `ImpulseRange`, `AngularImpulseRange`, and `FailSafeTime` (`float`, default 10s). On sleep, disables physics simulation on the settled mesh so it cannot be pushed again. Broadcasts `OnFailsafeDestroy` (with `EDiceType`) if a mesh hasn't settled within `FailSafeTime`. For two-mesh dice, Mesh2 receives an additional randomized offset impulse for varied separation. Has a `bWasKept` (`bool`, default `true`) — set to `false` by `UDiceSelectorManager` on the losing die of an advantage/disadvantage roll, for Blueprint visual feedback. `FRollResult` includes an `ABaseDiceActor* DiceActor` pointer (set in `GetRolledValue`) so the manager can identify which actor produced which result.
- **`UDiceData`** — `UPrimaryDataAsset` subclass. Stores per-die configuration data (mesh, faces, type, etc.).

### Chat/
- **`UChatBox`** — `UUserWidget` subclass. Tabbed chat container. BindWidgets: `TabBar` (`UHorizontalBox`), `ClosedChannelContainer` (`UVerticalBox`), `ChannelContainer` (`UWidgetSwitcher`), `EditableText` (`UEditableText`), `ChannelListButton` (`UButton`). EditAnywhere: `ChannelClass` (`TSubclassOf<UChatChannel>`), `TabClass` (`TSubclassOf<UChatTab>`), `ChatEntryClass` (`TSubclassOf<UChatEntry>`, passed through to channels), `ChannelListEntryClass` (`TSubclassOf<UChatChannelListEntry>`). Private: `Channels` (`TArray<UChatChannel*>`), `ActiveChannel` (`TObjectPtr<UChatChannel>`), `ChannelTabMap` (`TMap<UChatChannel*, UChatTab*>`, for O(1) tab lookup), `ClosedChannels` (`TSet<UChatChannel*>`). `NativeConstruct` gets `HUDComponentRef`, binds `EditableText->OnTextCommitted`, binds `ChannelListButton->OnClicked` → `OnChannelListButtonClicked`, collapses `ClosedChannelContainer`, and creates the Server channel via `CreateChannel({})`. `CreateChannel(TArray<FString> Participants)` builds the tab label ("Server" for empty, "@Name" for one participant, "@Name +N" for multiple), creates and configures both the `UChatChannel` and `UChatTab` widgets, wires `OnTabClicked` → `SwitchToChannel`, wires `OnTabClosed` → `CloseChannel`, calls `Tab->SetCloseable(!Participants.IsEmpty())` so the Server tab has no close button, adds both to the UI, and returns the channel. `SwitchToChannel(UChatChannel*)` must be `UFUNCTION()` for `AddDynamic` to work — re-enables the previously active tab, sets `ActiveChannel`, disables the new tab via `SetInteractable`, activates the channel in `ChannelContainer`, and calls `ClearNotification` on the new tab. `AddChatMessage(const FString& Message, TArray<FString> Participants, bool bIsSender)` finds or creates the matching channel (matched by `Num()` equality first, then participant-by-participant containment check to avoid partial matches), adds the message, then: if the channel is in `ClosedChannels`, calls `ReopenChannel` (auto-reopen on incoming message); else if `bIsSender`, calls `SwitchToChannel`; else if not the active channel, calls `ShowNotification`. `OnTextCommitted` parses the input string on Enter: splits on spaces, collects `@Name` tokens into `Recipients` and remaining words into the message body; if no `@` tokens were found and the active channel is private, automatically uses that channel's participants as recipients (minus the sender) so replies don't fall back to the Server channel; then calls `HUDComponentRef->SendChatMessageOnServer(FullMessage, Recipients)`. `FocusChat()` / `ExitChat()` manage input mode. `NativeOnMouseButtonDown` calls `FocusChat()` when not focused but passes through to Super so the click still reaches child widgets (tabs, buttons). `ExitChat()` also restores `bShowMouseCursor = true`. `AppendToInput(const FString& Text)` appends text to the current editable text value (used by `UGameplayHUDComponent::OnPlayerAddressClicked`). `GetActiveChannelParticipants()` returns `ActiveChannel->Participants`, or `{}` if no channel is active — used by `UGameplayHUDComponent::AddRollResultToChat` to route dice rolls to the active channel. Private `OnChannelListButtonClicked()` toggles `ClosedChannelContainer` between `Visible` and `Collapsed`. Private `CloseChannel(UChatChannel*)` adds the channel to `ClosedChannels`, collapses its tab widget, falls back to `Channels[0]` (Server) if the closed channel was active, then calls `RefreshChannelList`. Private `ReopenChannel(UChatChannel*)` removes from `ClosedChannels`, restores tab visibility, calls `SwitchToChannel`, calls `RefreshChannelList`, and collapses `ClosedChannelContainer`. Private `RefreshChannelList()` clears `ClosedChannelContainer` and repopulates it with one `UChatChannelListEntry` per closed channel; each entry's `OnEntryClicked` is bound to `ReopenChannel`. **Blueprint layout:** the tab row is a `UHorizontalBox` containing `TabBar` and `ChannelListButton` side by side; `ClosedChannelContainer` sits between that row and `ChannelContainer` in the outer `UVerticalBox`. Full structure: Border → SizeBox → VerticalBox → [HorizontalBox(TabBar, ChannelListButton)], ClosedChannelContainer, ChannelContainer, EditableText.
- **`UChatEntry`** — `UUserWidget` subclass. Single chat message row. Requires bound widget: `TextBlock` (`UTextBlock`). Exposes `Message` (`FString`, expose on spawn). Sets text in `NativeConstruct`.
- **`UChatChannel`** — `UUserWidget` subclass. Represents one chat channel (tab). Requires bound widget: `ScrollBox` (`UScrollBox`). Public: `ChatEntryClass` (`TSubclassOf<UChatEntry>`), `DisplayName` (`FString`, client-side label), `Participants` (`TArray<FString>`, internal participant list), `ScrollMultiplier` (`float`, default 60). `AddChatMessage(const FString& Message)` creates and appends a `UChatEntry`. `SetChatEntryClass(TSubclassOf<UChatEntry>)` lets `UChatBox` pass down the entry class at creation time. `Scroll(bool bUp)` adjusts `ScrollBox` offset by `ScrollMultiplier`, clamped to valid range.
- **`UChatTab`** — `UUserWidget` subclass. Clickable tab button in the tab bar. Requires bound widgets: `TabButton` (`UButton`), `CloseButton` (`UButton`), `TabLabel` (`UTextBlock`), `NotificationIndicator` (`UWidget`). Public: `OnTabClicked` delegate (`FOnTabClicked`, passes `UChatChannel*`), `OnTabClosed` delegate (`FOnTabClosed`, passes `UChatChannel*`), `SetChannel(UChatChannel*)`, `SetLabel(const FString&)`, `ShowNotification()`, `ClearNotification()`, `SetInteractable(bool)`, `SetCloseable(bool)`. `NativeConstruct` binds `TabButton->OnClicked` → private `OnTabButtonClicked` (broadcasts `OnTabClicked`) and `CloseButton->OnClicked` → private `OnCloseButtonClicked` (broadcasts `OnTabClosed`). `SetInteractable` calls `TabButton->SetIsEnabled` — used by `UChatBox::SwitchToChannel` to disable the active tab and re-enable the previous one. `SetCloseable` shows or hides `CloseButton` via Visibility (`Collapsed` when false) — the Server tab passes `false` so it cannot be closed.
- **`UChatChannelListEntry`** — `UUserWidget` subclass. A single row in the closed channel list panel. Requires bound widgets: `EntryButton` (`UButton`), `EntryLabel` (`UTextBlock`). Private: `Channel` (`TObjectPtr<UChatChannel>`). Public: `OnEntryClicked` delegate (`FOnEntryClicked`, passes `UChatChannel*`), `SetChannel(UChatChannel*)`. `SetChannel` stores the channel reference and sets `EntryLabel` text from `Channel->DisplayName`. `NativeConstruct` binds `EntryButton->OnClicked` → private `OnEntryButtonClicked` which broadcasts `OnEntryClicked` with the stored channel.

### PlayerList/
- **`UPlayerRow`** — `UUserWidget` subclass. Single row in the player list. Requires bound widgets: `NameLabel` (`UTextBlock`), `AddressButton` (`UButton`). Public: `OnAddressClicked` delegate (`FOnAddressClicked`, passes `const FString& PlayerName`). `SetPlayerName(const FString&)` sets the display label and stores the name. `NativeConstruct` binds `AddressButton->OnClicked` to a private handler that broadcasts `OnAddressClicked` with the stored name. Declares `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddressClicked, const FString&, PlayerName)`.
- **`UPlayerList`** — `UUserWidget` subclass. Collapsible scrollable list of connected players. Requires bound widgets: `ScrollBox` (`UScrollBox`), `ToggleButton` (`UButton`). EditAnywhere: `PlayerRowClass` (`TSubclassOf<UPlayerRow>`). Public: `OnAddressClicked` delegate (`FOnAddressClicked`), `PopulateList()`. `NativeConstruct` binds `ToggleButton`, calls `PopulateList()`, and collapses `ScrollBox`. `OnToggleButtonClicked` toggles `bIsExpanded`, calls `PopulateList()` when expanding (not at construct — `GameState` may not have all players yet), and sets `ScrollBox` visibility. `PopulateList()` clears children, iterates `GS->PlayerArray`, creates one `UPlayerRow` per player, and binds each row's `OnAddressClicked` to a local handler that bubbles `OnAddressClicked` upward.

### UI/
- **`UDragHandle`** — `UUserWidget` subclass. Hit-testable widget placed in the title bar of a `UDraggablePanel`. Captures mouse on press, forwards drag events to parent via `GetTypedOuter<UDraggablePanel>()`. Private: `bActive` (`bool`). Overrides `NativeOnMouseButtonDown` (sets `bActive`, calls `StartDrag`, captures mouse), `NativeOnMouseMove` (calls `UpdateDrag` while `bActive`, passes through to Super otherwise), `NativeOnMouseButtonUp` (clears `bActive`, calls `StopDrag`, releases capture).
- **`UResizeHandle`** — `UUserWidget` subclass. Hit-testable widget placed in the bottom-right corner of a `UDraggablePanel`. Same structure as `UDragHandle` but calls `StartResize`, `UpdateResize`, `StopResize`.
- **`UDraggablePanel`** — `UUserWidget` subclass. Wrapper widget that makes any content widget draggable and resizable at runtime. Must be placed inside a `UCanvasPanel` — caches its `UCanvasPanelSlot` in `NativeConstruct` for position/size updates. BindWidgets: `DragHandle` (`UWidget`), `ContentSlot` (`UNamedSlot`), `ResizeCorner` (`UWidget`), `TitleText` (`UTextBlock`). EditAnywhere: `TitleBarHeight` (`float`), `MinSize` (`FVector2D`), `PanelTitle` (`FText`). (`ResizeHandleSize` removed — resize zone size is now determined by the Blueprint size of `WBP_ResizeHandle`.) Private state: `DragOffset`, `ResizeStartMouse`, `ResizeStartSize` (`FVector2D`), `CanvasSlot` (`TObjectPtr<UCanvasPanelSlot>`). Public: `SetPanelTitle(FText)`, `StartDrag`, `UpdateDrag`, `StopDrag`, `StartResize`, `UpdateResize`, `StopResize` — all called by `UDragHandle`/`UResizeHandle`. Blueprint: `W_DraggablePanel` in `Content/UI/Utility/`.
- **`UDiceSelector`** — `UUserWidget` subclass. Requires bound widgets: `TypeText`, `CountText` (`UTextBlock`), `IncreaseButton`, `DecreaseButton` (`UButton`). Exposes `DiceClass` (`TSubclassOf<ABaseDiceActor>`), `DiceType` (`EDiceType`), and `DiceCount` (`int32`, visible/read-only). Button clicks bound in `NativeConstruct`. All logic is in C++ — the Blueprint exists only for layout and styling.
- **`UDiceSelectorManager`** — `UUserWidget` subclass. Requires bound widgets: `D4`, `D6`, `D8`, `D10`, `D12`, `D20`, `D100` (`UDiceSelector`), `NormalRollButton`, `AdvantageRollButton`, `DisadvantageRollButton`, `RollButton` (`UButton`). Exposes `SpawnVolume` (`TObjectPtr<ADiceSpawnVolume>`, set at runtime by `UGameplayHUDComponent`), `Impulse`, `AngularImpulse` (`FVector`), `ImpulseRange`, `AngularImpulseRange` (`float`), `TimeBeforeDestroyingDice` (`float`, default 5s), and `RollMode` (`EDiceRollMode`, visible/read-only) in the inspector. Selectors and `AdvantageButtons` arrays built in `NativeConstruct`. Supports three roll modes via `EDiceRollMode` (Normal, Advantage, Disadvantage). Mode buttons are enabled only when exactly one selector has `DiceCount == 1` — clicking a mode button disables itself and enables the others. In Advantage/Disadvantage mode, two dice of the selected type are spawned; `OnDiceRolledHandler` picks the higher (Advantage) or lower (Disadvantage) result, sets `bWasKept = false` on the losing actor, and broadcasts only the winning result. `bRollInProgress` is set to `true` before the spawning loop (not after) to prevent `ResetCount` from triggering `UpdateAdvantageButtonState` and resetting `RollMode` mid-roll. Each die's spawn location is chosen via `FMath::RandPointInBox(SpawnVolume->GetSpawnBox())` — dice are distributed randomly across the volume instead of all spawning at the same point. `RollDice` guards against a null `SpawnVolume` and logs a warning if it isn't set. `OnAllDiceRolled` broadcasts `TArray<FRollResult>` and `EDiceRollMode`. Broadcasts `OnDiceFailsafeDestroyed` (with `EDiceType`) when a die is lost to the failsafe.
- **`UTaskbarButton`** — `UUserWidget` subclass. A single toggle button in the taskbar. Requires bound widgets: `ToggleButton` (`UButton`), `WidgetLabel` (`UTextBlock`). Private: `TrackedWidget` (`TObjectPtr<UUserWidget>`). `SetTrackedWidget(UUserWidget*, FString Label)` stores the widget reference and sets the label text. `OnToggleClicked()` (`UFUNCTION`) toggles `TrackedWidget` between `Visible` and `Collapsed`: if `Collapsed` → set `Visible`; else → set `Collapsed`. `NativeConstruct` binds `ToggleButton->OnClicked` to `OnToggleClicked`.
- **`UTaskbar`** — `UUserWidget` subclass. Taskbar widget displayed at the bottom of the screen. Requires bound widget: `ButtonContainer` (`UHorizontalBox`). EditAnywhere: `TaskbarButtonClass` (`TSubclassOf<UTaskbarButton>`). `RegisterWidget(UUserWidget* Widget, FString Label)` creates a `UTaskbarButton` via `CreateWidget`, calls `SetTrackedWidget`, and adds the button to `ButtonContainer`. Called by `UGameplayHUDComponent::BeginPlay` for each HUD widget that should be toggleable.
- **`UGameplayHUDComponent`** — `UActorComponent` subclass. Manages HUD widget lifecycle and chat networking. Created in `AGameplayController` constructor. Caches `PlayerControllerRef` in `BeginPlay`. Sets `FInputModeGameAndUI` (with `DoNotLock`) and `bShowMouseCursor = true` at startup so UI widgets receive mouse events immediately. Creates `GameplayScreenClass` widget (local clients only), grabs `DiceSelectorManagerRef`, `ChatBoxRef`, `PlayerListRef`, and `TaskbarRef` via `GetWidgetFromName`. Binds to `OnAllDiceRolled` and `OnDiceFailsafeDestroyed`. After finding `DiceSelectorManagerRef`, uses `UGameplayStatics::GetActorOfClass` to locate the `ADiceSpawnVolume` in the level and assigns it to `DiceSelectorManagerRef->SpawnVolume` — logs a warning if none is found. Binds `PlayerListRef->OnAddressClicked` to `OnPlayerAddressClicked`, which calls `ChatBoxRef->AppendToInput("@" + PlayerName + " ")`. After finding `TaskbarRef`, calls private helper `FindAndRegisterPanel(FName WidgetName, FString Label)` for each panel — finds the named `UDraggablePanel` in the screen, registers it with the Taskbar, logs a warning if not found, and returns the ref. Stores results in `ChatPanel`, `DicePanel`, `PlayersPanel` (`TObjectPtr<UDraggablePanel>`). The Taskbar toggles the full panel (including the drag/resize wrapper), not just the inner content widget. Server RPC `SendChatMessageOnServer(const FString& Message, const TArray<FString>& Recipients)` — empty `Recipients` = broadcast to all; populated = private, builds a full `Participants` list (sender + all recipients) then routes only to players in that list. The `Participants` list (not the raw `Recipients`) is passed to `AddChatMessageOnOwningClient` so each client's channel includes all parties. Client RPC `AddChatMessageOnOwningClient(const FString& Message, const TArray<FString>& Recipients, bool bIsSender)` — passes all three params to `ChatBoxRef->AddChatMessage`. `AddRollResultToChat(TArray<FRollResult>, EDiceRollMode)` formats roll results as "[Player] Rolled:\n[value] on a [type]" per die; prepends "with Advantage" or "with Disadvantage" when `RollMode` is not Normal; calls `ChatBoxRef->GetActiveChannelParticipants()` to route the roll to the currently active channel rather than always broadcasting to all. `OnDiceFailsafeHandler` sends "[Player] lost a [type] to the void". Exposes public chat wrappers `FocusChat()`, `ExitChat()`, `ScrollChat(bool bUp)` for the controller to call. Includes `DiceSelectorManager.h`, `PlayerList.h`, and `Taskbar.h` directly.

### Pawns/
- **`AGameplayPawn`** — `APawn` subclass. Camera pawn for top-down view. Component hierarchy: `Root` (USceneComponent) → `Sphere` (UStaticMeshComponent) → `SpringArm` (USpringArmComponent) → `Camera` (UCameraComponent). All components public so `AGameplayController` can access `SpringArm->TargetArmLength` for speed calculation.

### PlayerControllers/
- **`AGameplayController`** — `APlayerController` subclass. Central hub for all player input and HUD management. Creates `HUDComponent` (`UGameplayHUDComponent`) in constructor. `BeginPlay` sets `bShowMouseCursor = true`. In `OnPossess`: casts possessed pawn to `AGameplayPawn`, sets up `UEnhancedInputLocalPlayerSubsystem` with `IMC_Gameplay` (priority 0), binds all input actions via `UEnhancedInputComponent`. **Movement**: `IA_CameraMove` (Vector2D) moves along the pawn's forward/right vectors scaled by `CalculateCameraMovementSpeed() * CurrentCameraSpeedMultiplier`; `Delta.Z` zeroed to keep movement flat on the ground plane regardless of camera pitch. `CalculateCameraMovementSpeed()` clamps `SpringArm->TargetArmLength / 100` between `MinCameraMovementSpeed` and `MaxCameraMovementSpeed`. **Sprint**: `IA_CameraSprint` (bool, Triggered+Completed) sets `CurrentCameraSpeedMultiplier` to `CameraSpeedMultiplier` when held, `1.f` when released. **Pan**: `IA_CameraPan` (bool, Started+Triggered+Completed) — Started/Completed toggle `bCanCameraMove`; Triggered calls `GetInputMouseDelta` and rotates the pawn: DeltaX→Yaw (unrestricted), DeltaY→Pitch (clamped to `[MinCameraPitch, MaxCameraPitch]`). `IA_CameraPanReset` resets pitch to -15.f while preserving Yaw. **Zoom**: `IA_CameraZoom` (float) adjusts `SpringArm->TargetArmLength` by `FMath::Sign(input) * ZoomSpeed`, clamped to `[MinZoomLength, MaxZoomLength]`. **Chat**: `IA_FocusChat`/`IA_ExitChat` swap `IMC_Chat` (priority 1) in/out and delegate to `HUDComponent`. `IA_ScrollChat` (float) passes sign to `HUDComponent->ScrollChat()`. **Editor validation**: `PostEditChangeProperty` (`#if WITH_EDITOR`) enforces positive minimums and min < max for all camera range properties — when min ≥ max, max is bumped up to preserve the min value. Camera properties: `MinCameraMovementSpeed` (5), `MaxCameraMovementSpeed` (20), `CameraSpeedMultiplier` (2), `MinCameraPitch` (-15), `MaxCameraPitch` (45), `CameraPanSpeedMultiplier` (5), `MinZoomLength` (100), `MaxZoomLength` (2500), `ZoomSpeed` (50). Private non-UPROPERTY: `CurrentCameraSpeedMultiplier` (runtime sprint state). `bCanCameraMove` is a plain `bool` (no UPROPERTY) to avoid Blueprint CDO override. Input assets: `IMC_Gameplay`, `IA_CameraMove`, `IA_CameraPan`, `IA_CameraPanReset`, `IA_CameraSprint`, `IA_CameraZoom`, `IA_FocusChat` (Gameplay Input); `IMC_Chat`, `IA_ExitChat`, `IA_ScrollChat` (Chat Input). All assigned in `PC_Gameplay` details panel.

### Utility/
- **`UFunctionLibrary`** — `UBlueprintFunctionLibrary`. General-purpose helper functions accessible from both C++ and Blueprint.

---

## Build Workflow

**Every session:**
1. Build in Visual Studio (Ctrl+Shift+B) — do this before opening the `.uproject`
2. Open the editor after the build succeeds

**When things get weird** (warnings persist, crashes, odd behavior):
1. Close the editor
2. Delete `Intermediate/` and `Binaries/` from the project root
3. Right-click `.uproject` → Generate Visual Studio project files
4. Build in Visual Studio
5. Open the editor

> Never let Unreal compile C++ on its own. Blueprint widgets that inherit from C++ classes will show "invalid parent class" warnings if the editor loads before C++ classes are fully registered.

---

## Build Configuration Notes

- All source subdirectories must be added to `PublicIncludePaths` in `ProjectIronTable.Build.cs`
- Uses `Path.Combine(ModuleDirectory, "FolderName")` — requires `using System.IO;` at the top of Build.cs
- This allows `#include "FileName.h"` without path prefixes from any folder in the module
- Without this, cross-folder includes fail even with correct relative paths
- Current registered folders: `Chat`, `Dice`, `UI`, `Utility`, `Pawns`, `PlayerControllers`, `PlayerList`

---

## Developer Notes / Known Gotchas

- Unreal's C++ class wizard generates incorrect include paths for files in subfolders — always verify after creation
- After adding new files or folders, regenerate VS project files (right-click `.uproject` → Generate Visual Studio project files) or new folders won't appear in the Content Browser
- `E1696` in Visual Studio is an IntelliSense error, not a real compiler error — build the project anyway to confirm
- `DiceData.h` is included in `BaseDiceActor.h` as `"DiceData.h"` with no path prefix, because both files are in `Dice/` and that folder is in `PublicIncludePaths`
- `OnComponentSleep` will not fire unless **Generate Wake Events** is checked on the Static Mesh component (Blueprint Details panel → Physics)
- `ExpectedDiceCount` and `PendingResults` must be reset at the start of `RollDice`
- `SpawnedDice` is cleared inside `DestroyDice()` after the timer fires — do not clear it earlier or dice won't be destroyed
- Use `int32` instead of `int` consistently throughout the project
- Pass `TArray` by const reference in `RollDice`
- `bMesh1Asleep` / `bMesh2Asleep` are runtime state — use `VisibleInstanceOnly` not `EditAnywhere`
- `IsMeshValid` and `GetFaceValue` are `const` methods — keep them that way
- `UFunctionLibrary::GetDiceName` is kept for reference from TTRPG_Sim — evaluate whether to replace with `UEnum::GetValueAsString()` later
- `UEnum::GetValueAsString()` returns `"EnumClass::ValueName"` — strip the prefix with `RightChop(str.Find("::") + 2)` before displaying to the user
- `UPhysicalMaterial` requires the `PhysicsCore` module in `Build.cs` — add it to `PublicDependencyModuleNames`
- Physical Materials (friction, restitution) live in `Content/Physics/Materials/` — assign via `UPhysicalMaterial*` UPROPERTY and apply with `SetPhysMaterialOverride` in `BeginPlay`
- `AddDynamic` requires the bound function to be a `UFUNCTION` — without it the binding silently fails
- Dice physics: settled meshes have `SetSimulatePhysics(false)` called in `OnMeshSleep` to prevent being pushed after landing
- `BindWidget` pointers are null at member declaration time — never initialize arrays or do work with them in the header. Always do so in `NativeConstruct`, after binding has occurred
- Actor Component C++ classes must have `Blueprintable` in their `UCLASS` specifier to appear as a reparent option in Blueprint. Without it the class will not show up in the picker even after a clean build and editor restart
- `FVector3f` is float precision (GPU/rendering use). World positions must use `FVector` (`TVector<double>`) — `FTransform` and `SpawnActor` will not accept `FVector3f`
- Includes that are only used in the `.cpp` belong in the `.cpp`, not the `.h` (e.g. `Kismet/KismetMathLibrary.h`)
- After adding a new module to `Build.cs`, regenerate VS project files so IntelliSense picks up the new include paths — autocomplete won't work until then even though E1696 isn't a real error
- `UPROPERTY` category names with spaces must use quoted strings: `Category = "My Category"` — unquoted names with spaces cause a UHT001 error
- `FInputModeUIOnly()` does NOT block Enhanced Input actions — it only affects legacy input and Slate viewport capture. To block game input while in chat, remove the movement IMC from the subsystem in `FocusChat()` and re-add it in `ExitChat()`
- `ETextCommit::OnCleared` fires when Escape is pressed in a focused `UEditableText` — the widget consumes Escape before Enhanced Input sees it, so handle exit-on-Escape in `OnTextCommitted` rather than via an input action
- `ETextCommit::OnUserMovedFocus` fires when the user clicks away from a focused `UEditableText` — use this to detect "click outside to exit"
- When `UEditableText` commits on Enter, it also immediately fires `OnUserMovedFocus` — if you want to stay in chat after sending, call `FocusChat()` at the end of the `OnEnter` block to re-establish focus
- Declaring `Type* MemberName = Cast<...>` in `BeginPlay` creates a local variable that shadows the member — the member is never assigned. Use `MemberName = Cast<...>` (no type prefix) to assign to the member
- For Actor Components, Enhanced Input setup goes in `BeginPlay`: get subsystem via `PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()`, bind actions via `Cast<UEnhancedInputComponent>(PC->InputComponent)->BindAction(...)`
- `UEnhancedInputLocalPlayerSubsystem` can be forward-declared in `.h` and included via `EnhancedInputSubsystems.h` in `.cpp`
- **Blueprint CDO overrides C++ constructor defaults** — any `UPROPERTY` on a Blueprint subclass uses the Blueprint's own saved CDO value, not the C++ constructor value. If a Blueprint was created before a default was set (or with a different value), it will keep its saved value forever. Fix: use `EditDefaultsOnly` and set the value in the Blueprint details panel, or set it explicitly in the constructor AND open the Blueprint to reset its CDO, or remove `UPROPERTY` if the value only needs C++ control
- `NewObject` cannot be used in constructors to create components — use `CreateDefaultSubobject`. `CreateDefaultSubobject` handles registration automatically; no `RegisterComponent` call needed
- For `APlayerController`, Enhanced Input setup (subsystem + action bindings) goes in `OnPossess`, not `BeginPlay` — `OnPossess` fires when the pawn is possessed and `InputComponent` is already valid at that point
- `Cast<UEnhancedInputComponent>(InputComponent)` silently returns null if **Project Settings → Input → Default Classes → Default Input Component Class** is not set to `EnhancedInputComponent` — verify this setting when moving Enhanced Input bindings from Blueprint to C++
- `FRotator(Pitch, Yaw, Roll)` — constructor order is NOT (X, Y, Z). Passing mouse DeltaX/DeltaY in the wrong positions scrambles rotation axes and causes violent camera bouncing
- `GetActorForwardVector()` has a Z component when the pawn is pitched — after computing movement delta from forward/right vectors, zero out `Delta.Z` to keep movement flat on the ground plane
- `PostEditChangeProperty` only fires in editor builds — always wrap with `#if WITH_EDITOR` / `#endif`. For runtime property validation, extract the logic into a shared `ValidateCameraSettings()` function callable from both `PostEditChangeProperty` and the runtime settings apply path. **This is required before shipping a settings menu** — without it, a player could set MinZoom > MaxZoom or a speed of 0, which would break camera movement silently
- `FMath::Sign(float)` returns `1.f`, `-1.f`, or `0.f` — useful for collapsing if/else scroll direction checks into a single clamped expression
- **`bRollInProgress` must be set before calling `ResetCount` on selectors** — `ResetCount` broadcasts `OnCountChanged` which triggers `UpdateAdvantageButtonState`. If `bRollInProgress` is still `false` at that point, the else branch fires and resets `RollMode` to Normal, breaking advantage/disadvantage rolls. Set `bRollInProgress = true` before the spawning loop, and reset to `false` in the else branch if nothing was spawned
- A `USTRUCT` with a raw pointer to a `UObject` subclass must use `UPROPERTY()` on that pointer — without it the GC can't track the reference. A forward declaration of the pointed-to class before the struct is sufficient for the header to compile when the full class is defined later in the same file
- `ADiceSpawnVolume` must be placed in the level for dice to spawn — `UGameplayHUDComponent::BeginPlay` uses `GetActorOfClass` to find it and hand the reference to `UDiceSelectorManager`. If it's not in the level, `RollDice` will log a warning and do nothing. Only one instance is needed.
- `UUserWidget` properties cannot be set by dragging level actors in the editor (widgets have no level context at edit time). Assign level actor references to widgets at runtime from a component that has access to both (e.g. `UGameplayHUDComponent::BeginPlay`)
- `SwitchToChannel` (and any function used as an `AddDynamic` target) must be marked `UFUNCTION()` — without it, the binding silently fails and tab clicks do nothing
- `UPlayerList::PopulateList` must be called on toggle open, not only in `NativeConstruct` — at construct time, `GameState->PlayerArray` may be empty because players haven't finished joining yet
- Channel matching in `UChatBox::AddChatMessage` must check `Participants.Num()` equality before checking content — without the size guard, a channel with a subset of the target participants can incorrectly match
- **`Participants` passed to `AddChatMessageOnOwningClient` must include the sender** — `SendChatMessageOnServer` receives only `Recipients` (the named addressees), but the sender must be added before passing to clients. Without `Participants.AddUnique(SenderName)`, channels created on recipients' clients will be missing the sender, causing a mismatch on future messages and a new duplicate channel being created each time
- **Taskbar toggle logic must treat any non-Collapsed visibility as "visible"** — `UUserWidget` default visibility is `SelfHitTestInvisible`, not `Visible`. Checking strictly for `ESlateVisibility::Visible` silently fails to collapse those widgets. Use `if (Collapsed) → Visible; else → Collapsed` instead of two separate equality checks
- **Always place Blueprint widget variants (e.g. `W_Taskbar`) in the gameplay screen, not the raw C++ class** — `GetWidgetFromName` will find the C++ class if placed directly, but its `BindWidget` members will be null since no Blueprint layout was set up, causing a crash on first access
- **`NativeOnMouseButtonDown` on a `UUserWidget` does not fire for clicks handled by child widgets** — child widgets (Borders, Buttons, etc.) consume mouse events before they bubble to the parent. Geometry-based hit checking on the parent is unreliable. Give each interactive zone (drag handle, resize corner) its own widget subclass that captures the mouse and calls back into the parent
- **`UDraggablePanel` must be placed inside a `UCanvasPanel`** — the panel caches `Cast<UCanvasPanelSlot>(Slot)` in `NativeConstruct`. If it's inside any other container (Overlay, VerticalBox, etc.), the cast returns null and all drag/resize calls do nothing silently
- **`UDraggablePanel` canvas slots must be anchored to the top-left** — resize logic adds a delta to the size using the original panel position as the fixed corner. With any other anchor, resizing one edge also shifts the opposite edge unexpectedly
- **Set `FInputModeGameAndUI` and `bShowMouseCursor = true` at HUD startup** — Unreal defaults to `GameOnly` input mode. Without this, mouse clicks never reach any UI widget. `UChatBox::ExitChat` must also restore `bShowMouseCursor = true` when restoring `GameAndUI` mode, or the cursor can be hidden and not come back
- **`UDragHandle`/`UResizeHandle` `NativeOnMouseMove` fires on hover, not just on drag** — guard with a `bActive` flag (set on `MouseButtonDown`, cleared on `MouseButtonUp`) to avoid calling `UpdateDrag`/`UpdateResize` while the user is merely hovering
- **`NativeOnMouseButtonDown` should pass through to Super after focus-on-click logic** — returning `FReply::Handled()` after calling `FocusChat()` consumes the click entirely, so child widgets (tab buttons, etc.) never receive it. The first click focuses; subsequent clicks interact. Fix: call `FocusChat()` then `return Super::NativeOnMouseButtonDown(...)` so both happen on the same click
- **Taskbar must toggle the `UDraggablePanel` wrapper, not the inner content widget** — registering `ChatBoxRef` directly with the Taskbar collapses the chat but leaves the drag/resize panel visible. Register the `UDraggablePanel` instances (`ChatPanel`, `DicePanel`, `PlayersPanel`) instead
- **Private channel replies require auto-routing** — if the user types a reply on a private tab without prefixing `@Name`, `Recipients` will be empty and `SendChatMessageOnServer` treats it as a broadcast. Detect this in `OnTextCommitted`: if Recipients is empty and ActiveChannel has participants, use those participants minus the sender as Recipients
- **Widgets inside `ContentSlot` only resize if their slots use Fill sizing** — changing the canvas slot size moves the panel boundary, but internal widgets sized with Auto will not reflow. Set container slots to Fill for elements that should grow with the panel; leave as Auto for elements that should stay fixed
- **`UButton` internal content padding is baked into the Style asset, not the slot or widget property** — the padding visible around a button's label cannot be adjusted via the Details panel slot settings. To remove or change it, expand the **Style** section on the Button in the Details panel and edit **Content Padding** there

---

## Roadmap

> This roadmap is organized by phase. Phases are not strictly time-boxed — each phase should be stable and functional before moving to the next. Notes and ideas are included for future reference.

---

### Phase 1 — Dice System (In Progress)
The foundation of the project. All dice logic, data, and UI.

- [x] Base dice actor (`ABaseDiceActor`)
- [x] Dice data asset (`UDiceData`)
- [x] Dice selector widget (`UDiceSelector`)
- [x] Dice selector manager (`UDiceSelectorManager`)
- [x] Individual die blueprints (D4, D6, D8, D10, D12, D20)
- [x] Dice meshes and materials (asset pack: *Dungeons of Dice* by NNJohn on Epic Fab)
- [x] Physics-based roll simulation
- [x] Roll result reading (face detection via normal dot product, verified working)
- [x] Roll result display in UI — complete (results broadcast to chat via OnAllDiceRolled → GameplayHUDComponent)
- [x] **Bug:** Rolling again before the previous roll despawns causes the second roll to vanish immediately and the first roll to remain permanently — fixed by cancelling the destroy timer and destroying existing dice at the start of `RollDice()`
- [x] Advantage / disadvantage support — rolls two of the selected die type, takes the higher (advantage) or lower (disadvantage) result; applies to any die type, not just D20
- [ ] Private / hidden rolls — any user can mark a roll as private, visible only to chosen recipients; roller can reveal at any time; non-recipients see a "[rolled privately]" indicator in chat
- [ ] Sound effects for dice rolls
- [ ] Visual effects for dice rolls
- [ ] Custom dice support (user-importable meshes and face values)
- [ ] Tune dice physics settings (mass, damping, impulse values) so rolls look and feel realistic
- [ ] Replace asset pack with original dice models (custom Blender meshes)

---

### Phase 2 — Core Game Setup
Establish the game framework and player interaction foundation.

- [x] Test Game Mode (`GM_Testing`)
- [x] Test Player Controller (`PC_Testing`)
- [x] Production Game Mode (`GM_Gameplay`)
- [x] Production Player Controller (`PC_Gameplay`)
- [x] HUD component (`BP_HUDComponent`) — complete, reparented to `UGameplayHUDComponent`
- [x] Player controller C++ class (`AGameplayController`) — owns all input, HUD component, camera movement
- [x] Camera pawn (`AGameplayPawn`) — top-down camera rig (Root → Sphere → SpringArm → Camera)
- [x] Camera movement polish (panning, zoom, bounds) — pan rotates pawn (yaw unrestricted, pitch clamped), zoom adjusts spring arm length, sprint multiplier, pan reset, editor property validation via PostEditChangeProperty. Runtime settings save system pending.
- [ ] Runtime camera settings menu (`USaveGame`-based) — exposes camera properties (speeds, zoom range, pan speed, pitch limits) to the player at runtime. **Requirements before shipping:** (1) extract `PostEditChangeProperty` validation logic into a shared `ValidateCameraSettings()` function callable at runtime; (2) call it when settings are applied so invalid values (min ≥ max, zero speeds) can never reach the camera movement code; (3) load saved settings in `BeginPlay` and apply before any input is processed.
- [x] Basic camera system (top-down / isometric view) — functional; may need revisiting as more systems are added
- [ ] Scene/session management (start, load, save)
- [ ] GM permissions system — GM can grant/revoke specific permissions to individual players at any time; permissions are enforced in code; specific permission types TBD when built
- [ ] Session player cap — default max 8 users (GM included); cap is enforced by default but can be removed; no hard engine limit
- [x] Private messaging — `@Name` syntax in chat input routes messages to specific recipients; server-side routing sends only to sender + named recipients; each private conversation gets its own tab (auto-created on first send); sender auto-switches to new tab; recipients see a notification indicator; tabs labeled `@P1 +N`; non-participants receive nothing
- [ ] Chat log persistence — full message and roll result history saved with session and restored on reload
- [ ] Shared notes — per-user notes documents, private by default; owner can share with specific users or all players with read or edit access; multiple users can edit simultaneously in real time; notes persist with session. **Note:** real-time collaborative editing requires careful networking design (OT or CRDT) — scope carefully when building

---

### Phase 3 — Maps
Virtual tabletop map system.

Two map formats are supported and can be used simultaneously:
- **Flat image** — user-imported image (PNG, JPG, etc.) used as map background; good for pre-drawn battle maps and scene art
- **Tile-based** — grid of placeable tiles assembled in-session; good for procedural dungeon building

- [ ] Flat image map support (import and display)
- [ ] Tile-based map system
- [ ] Grid overlay (square and hex, GM-toggled)
- [ ] User-importable map images and tile sets
- [ ] Fog of war (GM toggles revealed areas per-tile or per-region)
- [ ] Lighting and atmosphere controls
- [ ] Entity-based vision system — what a user sees is determined by what their controlled characters can perceive, not their role. The GM sees the full map but is still subject to vision-blocking effects (e.g., Sphere of Darkness) unless a controlled NPC can perceive through it. **Design note (future):** players and GMs need a way to see characters they control inside vision-blocking effects — likely via a character outline rendered through occluding geometry

---

### Phase 4 — Miniatures
Character and creature representation on the map.

**Scale:** Combat maps follow the active game system's movement rules (e.g., 1 tile = 5 feet in D&D 5e). Non-combat maps (world/region maps, scene art) have no enforced scale — the GM and players define distance in context.

- [ ] Base miniature actor/pawn (TBD — Actor vs Pawn)
- [ ] Miniature placement and movement on grid
- [ ] Scale system tied to active game system for combat maps; free scale for non-combat maps
- [ ] Default miniature options — game ships with built-in defaults; players can replace with custom assets (TBD: exact default design)
- [ ] Rigging system for miniature animations
- [ ] User-importable miniature meshes (format TBD when import pipeline is built)
- [ ] Mini labels (name, HP, status conditions)

---

### Phase 5 — D&D 5e / 2024 Game System
First full game system implementation.

- [ ] Character sheet — STR/DEX/CON/INT/WIS/CHA, proficiency bonus, saving throws, skill modifiers, HP (current/max/temp), hit dice, spell slots, conditions, inventory. Visible to owner, GM, and anyone the owner chooses to share with. GM can edit to some extent — specific limits TBD when built
- [ ] Initiative tracker — public turn order visible to all players; GM has a private staging list of combatants not yet in combat (hidden NPCs, reinforcements, ambushes) that can be inserted into the tracker at any time
- [ ] Initiative rolling — both manual (player rolls via dice UI) and automatic (pulled from character stats) supported
- [ ] Spell management — choosing spells, tracking slots by level, concentration, components, and spell effects; goal is to reduce player confusion
- [ ] Condition tracking — auto-applied when a triggering game event occurs; auto-apply is toggleable per table preference
- [ ] Attack rolls, saving throws, skill checks tied to character sheet stats
- [ ] Monster/NPC stat blocks
- [ ] Basic combat flow (action economy: action, bonus action, reaction, movement)

---

### Phase 6 — UI & Polish
Presentation layer, UI system, and immersive audio/visual.

- [ ] UI theming system — theme is decoupled from game system; each player can set their own theme; switching game systems can suggest a matching default theme but never forces it
- [ ] Draggable, resizable, toggleable UI panels — players have full control over HUD layout; layout persists per user across sessions
  - [x] Taskbar minimize system — `UTaskbar` + `UTaskbarButton` built; `UGameplayHUDComponent` registers Chat, Dice, and Players widgets; each button toggles its tracked widget between Visible and Collapsed
  - [x] Draggable and resizable panels — `UDraggablePanel`, `UDragHandle`, `UResizeHandle` built; Chat, Dice, and Players panels are all wrapped and independently moveable/resizable; single-corner resize (bottom-right); upgrade to 8-direction resize deferred
  - [x] Close and reopen private chat tabs — closing hides the tab (does not destroy it); closed channels accessible via `ChannelListButton` dropdown (`ClosedChannelContainer`); incoming message on a closed channel auto-reopens it
  - [ ] Panel layout persistence — save and restore panel positions/sizes per user across sessions
- [ ] Panel notification system — when activity occurs in a collapsed or hidden panel (new message, dice result, initiative change), a visible indicator appears so players don't miss events
- [ ] Replace default Unreal UI assets with custom art (buttons, panels, screens)
- [ ] Sound effects system (dice, movement, ambience)
- [ ] Visual effects system (spells, hits, status effects)
- [ ] User-importable sounds and VFX
- [ ] Music / ambience support
- [ ] User-uploadable music (players can load and play their own audio tracks during a session)

---

### Phase 7 — Custom Content & Extensibility
Allow users to bring their own assets and game rules.

**Asset library:** Imported assets belong to the user, not the session. Stored locally on the owner's machine (cloud storage is a future option). Assets persist across all sessions, campaigns, and game systems.

- [ ] Local asset library — persists across sessions; assets usable in any campaign or game system
- [ ] Custom asset import pipeline (sounds, maps, minis, effects)
- [ ] Custom dice import
- [ ] Plugin/mod support (TBD)
- [ ] Support for additional TTRPG systems beyond D&D 5e
- [ ] Player-to-player asset sharing (asset owners can optionally share custom files — minis, maps, sounds — with other players in a session)
- [ ] Cloud asset storage (optional, future)

---

### Multiplayer Architecture (Pending Research)
Networking decisions that require research before implementation.

**Known decisions:**
- GM role and session host role are separate — the GM does not need to be the machine hosting the server
- Default player cap: 8 users per session (GM included); cap can be removed; no hard engine limit
- When no GM is present (disconnect or absence), certain actions are locked (moving NPCs, editing stats); full disconnect policy TBD

**Open (needs research):**
- Listen server vs dedicated server
- Session discovery / join flow (direct IP, lobby, friend invite)

---

### Future / Backlog (Unscheduled)
Ideas to revisit later.

- Campaign management (sessions, notes, loot)
- Integrated rulebook reference
- AI game master assistant
- Mobile or web companion app

---

*Last updated: 2026-03-31* — Added `UChatChannelListEntry`. Updated `UChatTab` with close button, `OnTabClosed` delegate, and `SetCloseable`. Updated `UChatBox` with close/reopen channel logic, closed channel list panel (`ClosedChannelContainer`, `ChannelListButton`), `ClosedChannels` set, `CloseChannel`, `ReopenChannel`, `RefreshChannelList`, `OnChannelListButtonClicked`, and auto-reopen on incoming message. Checked off close/reopen tabs roadmap item. Added `UButton` style padding gotcha.

---

Copyright 2026 Abraham Elfenbaum. All Rights Reserved.

