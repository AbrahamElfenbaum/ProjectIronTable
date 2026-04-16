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
├── AssetLibrary/      — Asset library screen (AssetLibraryScreen)
├── CampaignBrowser/   — Campaign browser screen (CampaignBrowserScreen)
├── CampaignManager/   — Campaign manager widget classes (GameTypeButton, CampaignCard, CampaignManagerScreen)
├── Chat/              — Chat widget classes (ChatBox, ChatEntry, ChatChannel, ChatTab, ChatChannelListEntry)
├── Components/        — Actor component classes (SessionHUDComponent, MainScreenHUDComponent)
├── Dice/              — Dice actors and data assets
├── GameInstances/     — Game instance class (SessionInstance)
├── GameModes/         — Game mode classes (SessionGameMode)
├── GameStates/        — Game state classes (SessionGameState)
├── Pawns/             — Pawn classes
├── PlayerControllers/ — Player controller classes
├── PlayerList/        — Player list widget classes (PlayerList, PlayerRow)
├── PlayerStates/      — Player state classes (SessionPlayerState)
├── SaveLoad/          — Save game classes (PanelLayoutSave, CameraSettingsSave, CampaignManagerSave, SessionSave, PlayerSave)
├── Settings/          — Settings widget classes (CameraSettingsPanel, SettingsScreen)
├── UI/                — Non-chat widget classes (no HUD components; HomeScreen and BaseScreen live here)
└── Utility/           — Function libraries and general-purpose helpers
```

### Content Browser (`Content/`)
```
Content/
├── Blueprints/
│   ├── Core/
│   │   ├── GameModes/          — GM_Testing, GM_Session, GM_MainScreen
│   │   ├── PlayerControllers/  — PC_Testing, PC_Session, PC_MainScreen
│   │   └── Components/         — BP_SessionHUDComponent, BP_HomeScreenHUDComponent
│   ├── Dice/
│   │   ├── A_BaseDiceActor     — Base dice actor Blueprint
│   │   └── DiceActors/         — Individual die Blueprints (A_D4, A_D6, etc.)
│   └── Pawns/
│       └── P_Session      — Camera pawn Blueprint
├── Data/DataAssets/Dice/       — DA_ prefixed dice data assets
├── Input/
│   ├── Session/                — IMC_Session, IA_CameraMove, IA_CameraPan, IA_CameraPanReset, IA_CameraSprint, IA_CameraZoom, IA_FocusChat
│   └── Chat/                   — IMC_Chat, IA_ExitChat, IA_ScrollChat
├── UI/
│   ├── Dice/                   — WE_DiceSelector, W_DiceSelectorManager
│   ├── Chat/                   — W_ChatBox, WE_ChatChannel, WE_ChatTab, WE_ChatEntry
│   ├── PlayerList/             — W_PlayerList, WE_PlayerRow
│   ├── CampaignManager/        — WE_GameTypeButton, WE_CampaignCard
│   ├── Screens/                — S_SessionScreen, S_HomeScreen, S_MainScreen, S_SettingsScreen, S_CampaignManagerScreen, S_CampaignBrowserScreen, S_AssetLibraryScreen
│   ├── Settings/               — WE_SettingsSlider
│   │   └── Panels/             — WE_CameraSettingsPanel
│   ├── HUD/                    — W_Taskbar, WE_TaskbarButton
│   ├── Utility/                — W_DraggablePanel, WE_DragHandle, WE_ResizeHandle, WE_ContextMenu, WE_ContextMenuButton
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

Defines the spawn area for dice. `USessionHUDComponent` finds it at runtime and passes it to `UDiceSelectorManager`.

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

**Config (EditAnywhere):** `ChannelClass`, `TabClass`, `ChatEntryClass`, `ChannelListEntryClass`, `ContextMenuClass`

**Blueprint layout:** `Border → SizeBox → VerticalBox → [HBox(TabBar, ChannelListButton)], ClosedChannelContainer, ChannelContainer, EditableText`

**Key Methods:**
- `CreateChannel(TArray<FString> Participants)` — creates channel + tab, wires delegates, returns channel. Empty participants = Server tab (not closeable).
- `FindOrCreateChannel(TArray<FString> Participants)` — builds a sorted participant key via `UFunctionLibrary::MakeParticipantKey`, compares against each channel's key, and returns the match or creates a new channel. Used by both `AddChatMessage` and the chat log restore loop.
- `SwitchToChannel(UChatChannel*)` — activates channel, clears notification and input
- `AddChatMessage(Message, Participants, bIsSender)` — routes to correct channel via `FindOrCreateChannel`; auto-reopens closed channels on incoming message; shows notification if not active
- `FocusChat()` / `ExitChat()` — manage input mode. `ExitChat` does **not** clear the input field.
- `AppendToInput(FString)` — appends text to current input value
- `GetActiveChannelParticipants()` — returns active channel's participants, or `{}` for Server
- `TrySendPrivateRollMessage()` — if `@Name` tokens in input, sends them as a message and clears input; noop otherwise
- `ParseMentions(Message, OutRecipients, OutBody)` *(private)* — splits a message string on spaces; words prefixed with `@` are stripped and added to `OutRecipients`; the rest join into `OutBody`. Called by both `OnTextCommitted` and `TrySendPrivateRollMessage`.

> **Note:** Channel matching checks `Participants.Num()` equality before content — required to avoid partial matches.
>
> **Note:** `SwitchToChannel` must be `UFUNCTION()` for `AddDynamic` to work.
>
> **Note:** `ExitChat` must not clear the input — players type `@Names` before clicking Roll.
>
> **Note:** Slate fires `OnEnter` then immediately `OnUserMovedFocus` after the player hits Enter. The `bPendingRefocus` flag absorbs the spurious focus-loss event so `ExitChat` isn't called right after `FocusChat`.

---

#### UChatChannel
**Type:** `UUserWidget` | **Blueprint:** `WE_ChatChanel`

Represents one channel/tab's message list.

**Bound Widgets:** `ScrollBox` (`UScrollBox`)

**Config (EditAnywhere):** `ChatEntryClass`, `ScrollMultiplier` (default 60)

**State (VisibleAnywhere):** `DisplayName` (client-local label — safe to rename; routing uses `Participants`), `Participants` (`TArray<FString>`)

**Key Methods:**
- `AddChatMessage(FString)` — creates and appends a `UChatEntry`; logs warning if `ChatEntryClass` is null
- `RestoreMessage(SenderName, Message)` — recreates a saved message entry directly in the scroll box, bypassing routing and notification logic. Used by `USessionHUDComponent` to restore chat log on session load.
- `Scroll(bool bUp)` — adjusts scroll offset by `ScrollMultiplier`

---

#### UChatTab
**Type:** `UUserWidget` | **Blueprint:** `WE_ChatTab`

Clickable tab button in the tab bar. Supports left-click to switch channel, right-click to open a context menu with Rename and Close options, and inline rename via `UEditableText`.

**Bound Widgets:** `TabButton` (`UButton`), `TabLabel` (`UTextBlock`), `EditLabel` (`UEditableText`), `NotificationIndicator` (`UWidget`)

**Key Methods:**
- `GetChannel()` — returns the `UChatChannel*` this tab represents
- `SetInteractable(bool)` — enables/disables `TabButton` (active tab is disabled so it can't be re-clicked)
- `ShowNotification()` / `ClearNotification()`
- `EnterRenameMode()` — hides `TabLabel`, shows and focuses `EditLabel` pre-populated with the current label text. Commit via Enter saves the name; any other commit method (focus loss, Escape) cancels.

**Delegates:** `OnTabClicked` (→`UChatChannel*`), `OnTabRightClicked` (→`UChatChannel*`), `OnTabRenamed` (→`UChatTab*, FString NewName`)

> **Note:** `OnTabRenamedCompleted` only broadcasts `OnTabRenamed` if `CommitMethod == OnEnter && !Text.IsEmpty()` — focus loss or Escape cancels silently.

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

### AssetLibrary/

#### UAssetLibraryScreen
**Type:** `UBaseScreen` | **Blueprint:** `S_AssetLibraryScreen`

Stub screen for the asset library. Inherits all back-navigation behavior from `UBaseScreen`. No additional logic yet.

---

### CampaignBrowser/

#### UCampaignBrowserScreen
**Type:** `UBaseScreen` | **Blueprint:** `S_CampaignBrowserScreen`

Stub screen for the public campaign browser. Inherits all back-navigation behavior from `UBaseScreen`. No additional logic yet.

---

### CampaignManager/

#### UGameTypeButton
**Type:** `UUserWidget`

Tab button representing a single game type in the Campaign Manager screen. Displays the game type name and is greyed out (non-interactable) when no campaigns exist for that type. Visually indicates the active tab via background color.

**Bound Widgets:** `GameTypeTab` (`UButton`), `GameTypeLabel` (`UTextBlock`)

**Private State:** `SelectedTabColor` (`FLinearColor`), `UnselectedTabColor` (`FLinearColor`) — set by `UCampaignManagerScreen` at creation time via `SetTabColors`

**Delegates:**
- `OnGameTypeSelected` (`FOnGameTypeSelected(const FString& GameType)`, BlueprintAssignable) — broadcast on click, passing the game type key

**Key Methods:**
- `SetLabel(const FString& Label)` — sets the button label text
- `SetInteractable(bool bInteractable)` — enables or disables the button
- `SetTabColors(const FLinearColor& InSelected, const FLinearColor& InUnselected)` — stores the two colors; must be called before `SetSelected`
- `SetSelected(bool bSelected)` — applies `SelectedTabColor` or `UnselectedTabColor` to the button background
- `GetLabel() const` → `FString` — returns the current label text

---

#### UCampaignCard
**Type:** `UUserWidget`

Displays one campaign entry. Stores campaign ID and game type as private members; launched via button click.

**Bound Widgets:** `LaunchCampaignButton` (`UButton`), `CampaignTitle`, `LastPlayedDate`, `NumberOfPlayers` (`UTextBlock`)

**Private State:** `CampaignID` (`FGuid`), `GameType` (`FString`) — set via `SetCampaignData`

**Delegates:**
- `OnCampaignSelected` (`FOnCampaignSelected(const FGuid& CampaignID, const FString& GameType)`, BlueprintAssignable)

**Key Methods:**
- `SetCampaignTitle(const FString&)`, `SetLastPlayedDate(const FString&)`, `SetNumberOfPlayers(int32)`, `SetCampaignData(const FGuid&, const FString&)`

---

#### UCampaignManagerScreen
**Type:** `UBaseScreen` | **Blueprint:** `S_CampaignManagerScreen`

Root widget for the Campaign Manager. Inherits back-navigation from `UBaseScreen`. Loads saved campaigns, builds game type tab buttons, and populates a campaign card grid filtered by the selected game type. Supports a fake-data mode for testing without a real save.

**Config:**
- `GameTypeButtonClass` (`TSubclassOf<UGameTypeButton>`)
- `CampaignCardClass` (`TSubclassOf<UCampaignCard>`)
- `SelectedTabColor` / `UnselectedTabColor` (`FLinearColor`) — passed to each button via `SetTabColors`; must have alpha > 0 or buttons appear invisible
- `bUseFakeData` (`bool`) — when true, `Init` uses hardcoded test data instead of the save game

**Bound Widgets:** `NewCampaignButton` (`UButton`), `GameTypeTabBar` (`UScrollBox`), `CampaignGrid` (`UWrapBox`), `CampaignScroll` (`UScrollBox`) *(BackButton inherited from UBaseScreen)*

**Private State:** `SelectedGameType` (`FString`), `CampaignData` (`TObjectPtr<UCampaignManagerSave>`, `UPROPERTY()`), `ActiveButtons` (`TArray<UGameTypeButton*>`, `UPROPERTY()`) — only buttons with at least one campaign

**Key Methods:**
- `virtual void Init() override` — branches on `bUseFakeData`; either creates a temporary `UCampaignManagerSave` via `NewObject` and populates it with `BuildFakeData()`, or loads from the `"CampaignManager"` save slot. Creates one `UGameTypeButton` per game type, tracks buttons with campaigns in `ActiveButtons`, populates the grid with the first available type, calls `SetSelectedGameButton()` after the loop.
- `SetSelectedGameButton()` — iterates `ActiveButtons` and calls `SetSelected` based on `SelectedGameType`
- `BuildFakeData() const` — returns a `TMap<FString, FCampaignList>` with DnD5e (20), Pathfinder2e (4), CallOfCthulhu (2), Starfinder (2), VtM (1), Shadowrun/WFRP/CyberpunkRED/Mothership (0 each)

**Handlers:**
- `OnGameTypeSelected(const FString&)` — updates `SelectedGameType`, refreshes grid, calls `SetSelectedGameButton`
- `OnCampaignSelected(const FGuid&, const FString&)` — placeholder; will launch the selected campaign

> **Note:** The campaign grid (`UWrapBox`) must be inside a `UScrollBox` to support vertical scrolling when cards overflow.
>
> **Note:** `SelectedTabColor` and `UnselectedTabColor` default to alpha 0 — set alpha > 0 in the Blueprint defaults or tab buttons will be invisible.

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

#### UCampaignManagerSave
**Type:** `USaveGame` | **Slot:** `"CampaignManager"`, index 0

Persists all campaigns grouped by game type.

**Supporting types:**
- `FCampaignRecord` (`USTRUCT`) — one campaign entry: `CampaignID` (`FGuid`), `CampaignName` (`FString`), `LastPlayed` (`FDateTime`), `NumberOfPlayers` (`int32`), `SessionIDs` (`TArray<FGuid>`) — ordered list of session IDs belonging to this campaign; used as the index to locate sessions without filesystem scanning
- `FCampaignList` (`USTRUCT`) — wrapper struct holding `TArray<FCampaignRecord> Campaigns`; required because `TMap<K, TArray<V>>` cannot be a `UPROPERTY`

**Fields:** `CampaignRecords` (`TMap<FString, FCampaignList>`) — keyed by game type string (e.g. `"DnD5e"`)

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

#### UPlayerSave
**Type:** `USaveGame` | **Slot:** `"PlayerIdentity"`, index 0

Stores the player's permanent identity GUID. Created on first launch by `USessionInstance::Init` with a newly generated GUID; reloaded every session after. The `PlayerID` is appended to the `ServerTravel` URL as `?PlayerID=<guid>` and read by `ASessionGameMode::PostLogin` via `UGameplayStatics::ParseOption`.

**Fields:**
| Field | Type | Notes |
|---|---|---|
| `PlayerID` | `FGuid` | Permanent player identity. Generated once; never changes. |

---

#### USessionSave
**Type:** `USaveGame` | **Slot:** `"Session_{SessionID}"`

Per-session save file. One instance per game session. `UCampaignManagerSave` is the authoritative index — `FCampaignRecord.SessionIDs` lists all sessions for a campaign. To open a campaign, read its `SessionIDs` and load the most recent by `LastSaved`.

**Fields:**
| Field | Type | Replicated | Notes |
|---|---|---|---|
| `GameTypeID` | `FString` | — | Game system key (e.g. `"DnD5e"`); matches `UCampaignManagerSave` key |
| `CampaignID` | `FGuid` | — | Campaign this session belongs to |
| `SessionID` | `FGuid` | — | Unique session identity; used to build save slot name |
| `HostPlayerID` | `FGuid` | — | Server Owner. One per session, non-transferable |
| `GMPlayerIDs` | `TArray<FGuid>` | — | All active GMs. Multiple supported; transferable; default = campaign creator |
| `PlayerIDs` | `TArray<FGuid>` | — | All non-GM players |
| `LastSaved` | `FDateTime` | — | Used to sort sessions when loading a campaign (most recent first) |
| `ChatLog` | `TMap<FString, FChatLogRecord>` | — | Keyed by sorted, pipe-joined participant names (e.g. `"Alice\|Bob"`). Empty string key = Server channel. `FChatLogRecord` wraps `TArray<FChatMessageRecord>`; each record holds `SenderName` and `Message` body. |
| `ChatTabNames` | `TMap<FString, FString>` | — | Keyed by sorted, pipe-joined participant names (same key format as `ChatLog`). Value is the user-assigned display name for that tab. Persisted on rename; restored by `USessionHUDComponent::BeginPlay` after the chat log is restored. |

`FChatMessageRecord` and `FChatLogRecord` are declared in `SessionSave.h`.

---

### GameInstances/

#### USessionInstance
**Type:** `UGameInstance` | **Set in:** Project Settings → Maps & Modes → Game Instance Class

Persistent game instance that survives level transitions. Carries session context and the player's permanent identity. On startup (`Init`), loads `UPlayerSave` from disk and stores `PlayerID`; creates and saves a new one if none exists. `CampaignID` and `SessionID` are set before `ServerTravel` and read in `InitGame`.

**Fields:**
| Field | Type | Notes |
|---|---|---|
| `CampaignID` | `FGuid` | Campaign context — not used to locate the save file, but carried for bookkeeping |
| `SessionID` | `FGuid` | Used to build the save slot name: `"Session_{SessionID}"` |
| `PlayerID` | `FGuid` | Persistent player identity loaded from `UPlayerSave` on startup. Appended to travel URL as `?PlayerID=<guid>`. |

**Public API:** `GetCampaignID`, `SetCampaignID`, `GetSessionID`, `SetSessionID`, `GetPlayerID`, `SetPlayerID`

---

### GameStates/

#### ASessionGameState
**Type:** `AGameStateBase` | **Assigned in:** `GM_Session`

Runtime session state replicated to all clients. Mirrors `USessionSave` at runtime — loaded from disk on session start by `ASessionGameMode::InitGame`, written back on save. Server is authoritative; all fields have public getters and setters.

**Fields:**
| Field | Type | Replicated | Notes |
|---|---|---|---|
| `GameTypeID` | `FString` | Yes | Game system identifier |
| `CampaignID` | `FGuid` | No | Server bookkeeping only |
| `SessionID` | `FGuid` | No | Server bookkeeping only |
| `HostPlayerID` | `FGuid` | Yes | Server Owner identity |
| `GMPlayerIDs` | `TArray<FGuid>` | Yes | All active GMs |
| `PlayerIDs` | `TArray<FGuid>` | Yes | All non-GM players |
| `LastSaved` | `FDateTime` | No | Save logic only |

> Uses `AGameStateBase`, not `AGameState` — avoids match state logic and built-in `PlayerArray` that are designed for arena/shooter games.

---

### PlayerStates/

#### ASessionPlayerState
**Type:** `APlayerState` | **Assigned in:** `GM_Session`

Per-player runtime state replicated to all clients. Holds role flags derived from `ASessionGameState` — set by the server in `PostLogin`. Not persisted to disk; repopulated each session from `USessionSave`.

**Fields:**
| Field | Type | Replicated | Notes |
|---|---|---|---|
| `SessionPlayerID` | `FGuid` | Yes | Persistent player identity GUID — distinct from engine's built-in `int32 PlayerID` |
| `bIsGM` | `bool` | Yes | Derived from `ASessionGameState::GMPlayerIDs` |
| `bIsHost` | `bool` | Yes | Derived from `ASessionGameState::HostPlayerID` |

**Public API:** `GetSessionPlayerID`, `SetSessionPlayerID`, `GetIsGM`, `SetIsGM`, `GetIsHost`, `SetIsHost`

> Do **not** name the field `PlayerID` — shadows `APlayerState::PlayerID` (`int32`), causing a compiler error. Use `SessionPlayerID`.

> `SessionPlayerID` is set in `PostLogin` by parsing `OptionsString` via `UGameplayStatics::ParseOption(OptionsString, "PlayerID")`. The client appends the ID to the travel URL in `UCampaignManagerScreen::OnCampaignSelected`.

---

### GameModes/

#### ASessionGameMode
**Type:** `AGameModeBase` | **Blueprint:** *(pending)*

Server authority hub for gameplay sessions. Manages session init, player login/logout, and role assignment.

**`InitGame`:** Reads `SessionID` from `USessionInstance`, loads the matching `USessionSave` slot (`"Session_{SessionID}"`), and pushes all fields into `ASessionGameState`. Early returns with warnings on null instance, missing save, or null GameState.

**`PostLogin`:** Parses `PlayerID` from `OptionsString` via `UGameplayStatics::ParseOption`, calls `FGuid::Parse` to convert to `FGuid`, sets it on `ASessionPlayerState` via `SetSessionPlayerID`. Then sets `bIsHost` and `bIsGM` by comparing against `ASessionGameState` data. Adds the player to `GMPlayerIDs` or `PlayerIDs` in GameState using `AddUnique`. Warns and returns early if options string is empty or GUID parse fails.

**`Logout`:** Casts `AController*` to `APlayerController*`, gets their `SessionPlayerState`, removes their `SessionPlayerID` from `GMPlayerIDs` or `PlayerIDs` based on `bIsGM`.

> Uses `AGameModeBase`, not `AGameMode` — avoids match state logic not needed for TTRPG sessions.

---

### Components/

#### UMainScreenHUDComponent
**Type:** `UActorComponent` | **Blueprint:** `BP_HomeScreenHUDComponent` (rename pending)
**Owner:** `AMainScreenController`

Handles screen-level navigation only. Creates the root `S_MainScreen` widget, gets refs to all five screens, calls `Init()` on each, and wires their navigation delegates to the `ScreenSwitcher`.

**Config:**
- `MainScreenClass` (`TSubclassOf<UUserWidget>`) — **must be set on the component instance inside `PC_MainScreen`**, not on a standalone component Blueprint

**Widget names it searches for (must match exactly):**
- `ScreenSwitcher` (`UWidgetSwitcher`) — root switcher on `S_MainScreen`
- `HomeScreen` (`UHomeScreen`) — index 0
- `CampaignManagerScreen` (`UCampaignManagerScreen`) — index 1
- `CampaignBrowserScreen` (`UCampaignBrowserScreen`) — index 2
- `AssetLibraryScreen` (`UAssetLibraryScreen`) — index 3
- `SettingsScreen` (`USettingsScreen`) — index 4

**Handlers:**
- `OnCampaignManagerClicked` → `SetActiveWidgetIndex(1)` — bound to `UHomeScreen::OnCampaignManagerRequested`
- `OnCampaignBrowserClicked` → `SetActiveWidgetIndex(2)` — bound to `UHomeScreen::OnCampaignBrowserRequested`
- `OnAssetLibraryClicked` → `SetActiveWidgetIndex(3)` — bound to `UHomeScreen::OnAssetLibraryRequested`
- `OnSettingsClicked` → `SetActiveWidgetIndex(4)` — bound to `UHomeScreen::OnSettingsRequested`
- `OnBackClicked` → `SetActiveWidgetIndex(0)` — bound to `OnBackRequested` on all four non-home screens

> Does **not** own any button refs, slider refs, or save/load logic — all of that lives in the individual screen classes.

---

#### USessionHUDComponent
**Type:** `UActorComponent` | **Replicated:** yes
**Owner:** `ASessionController`

Manages the session HUD lifecycle and all chat networking.

**Config:**
- `SessionScreenClass` (`TSubclassOf<UUserWidget>`)

**Widget names it searches for (must match):** `DiceSelectorManager`, `ChatBox`, `PlayerList`, `Taskbar`, `DicePanel`, `ChatPanel`, `PlayersPanel`

**Key Methods:**
- `FindAndRegisterPanel(WidgetName, Label)` — finds `UDraggablePanel`, registers with taskbar, assigns ID, binds layout save delegates. Populated into `Panels` array at BeginPlay.
- `SavePanelLayout()` — iterates `Panels` array and writes each panel's layout to `"PanelLayout"` save slot
- `LoadPanelLayout()` — loads and applies saved panel layout to each panel in `Panels` array on startup
- `FocusChat()` / `ExitChat()` / `ScrollChat(bool)` — delegate to `ChatBoxRef`
- `OnRollInitiated()` — calls `TrySendPrivateRollMessage` before dice spawn

**RPCs:**
- `SendChatMessageOnServer` (Server, Reliable) — resolves sender name, builds participant list, routes to each player's HUD component via `AddChatMessageOnOwningClient`. After routing, also persists the message to `USessionSave`: parses sender/body from the formatted message string, builds a sorted pipe-joined participant key, and saves the record via `FindOrAdd` on `ChatLog`.
- `AddChatMessageOnOwningClient` (Client, Reliable) — delivers message to `ChatBoxRef`

**Chat log restore (BeginPlay):** After `ChatBoxRef` is initialized, loads `USessionSave` and iterates `ChatLog`. For each entry, splits the key on `|` to recover the participant list, calls `ChatBoxRef->FindOrCreateChannel(Recipients)`, then calls `Channel->RestoreMessage` for each saved message. This runs only for the local player controller.

> **Note:** Always place the Blueprint variant (`W_Taskbar`, etc.) in the screen widget — placing the raw C++ class causes null `BindWidget` crashes on first access.

---

### Settings/

#### UCameraSettingsPanel
**Type:** `UUserWidget` | **Blueprint:** `WE_CameraSettingsPanel` (`Content/UI/Settings/Panels/`)

Self-contained camera settings panel. Owns all 9 `USettingsSlider` refs, the Apply button, and the Reset button. Handles save/load of `UCameraSettingsSave` from the `"CameraSettings"` slot independently.

**Bound Widgets:** `MinCamSpeed`, `MaxCamSpeed`, `CamSpeedMultiplier`, `MinPitch`, `MaxPitch`, `PanMultiplier`, `MinZoom`, `MaxZoom`, `ZoomSpeed` (`USettingsSlider`), `ApplyButton`, `ResetButton` (`UButton`)

**Key Methods:**
- `Init()` — populates the `SettingsSliders` TArray, binds Apply/Reset button delegates, loads `"CameraSettings"` save slot (or applies defaults on first launch)

**Runtime:** `SettingsSliders` (`TArray<USettingsSlider*>`, `UPROPERTY()`) — all 9 refs in order for batch reset.

**Handlers:**
- `OnApplyClicked` — creates `UCameraSettingsSave`, reads all 9 slider values, calls `SaveGameToSlot("CameraSettings", 0)`
- `OnResetClicked` — calls `ResetToDefault()` on all sliders, then `OnApplyClicked`

---

#### USettingsScreen
**Type:** `UBaseScreen` | **Blueprint:** `S_SettingsScreen`

Root settings screen widget. Inherits back button, `OnBackRequested` delegate, and `NativeConstruct` binding from `UBaseScreen`. Hosts a `UWidgetSwitcher` for future panel navigation.

**Bound Widgets:** `PanelSwitcher` (`UWidgetSwitcher`), `CameraSettingsPanel` (`UCameraSettingsPanel`) *(BackButton inherited from UBaseScreen)*

**NativeConstruct:** calls `Super::NativeConstruct()` (binds back button), then calls `CameraSettingsPanel->Init()`

---

### UI/

#### UHomeScreen
**Type:** `UUserWidget` | **Blueprint:** `S_HomeScreen`

Home screen widget. Owns all five home screen buttons and exposes delegates for screen-level navigation.

**Bound Widgets:** `CampaignManagerButton`, `CampaignBrowserButton`, `AssetLibraryButton`, `SettingsButton`, `QuitButton` (`UButton`)

**Delegates:**
- `OnCampaignManagerRequested` (BlueprintAssignable) — bound by `UMainScreenHUDComponent` → switches to Campaign Manager screen
- `OnCampaignBrowserRequested` (BlueprintAssignable) — bound by `UMainScreenHUDComponent` → switches to Campaign Browser screen
- `OnAssetLibraryRequested` (BlueprintAssignable) — bound by `UMainScreenHUDComponent` → switches to Asset Library screen
- `OnSettingsRequested` (BlueprintAssignable) — bound by `UMainScreenHUDComponent` → switches to Settings screen

**Key Methods:**
- `Init()` — binds all button delegates

**Direct handlers (no delegate):**
- Quit → `QuitGame` directly

---

#### UBaseScreen
**Type:** `UUserWidget`

Shared base class for all main screen widgets. Provides a common back button, `OnBackRequested` delegate, and a virtual `Init()` hook so subclasses only need to override what they actually use.

**Bound Widgets (protected):** `BackButton` (`UButton`) — must be named exactly `BackButton` in every child Blueprint

**Delegates:**
- `OnBackRequested` (`FOnBackRequested`, BlueprintAssignable) — broadcast when `BackButton` is clicked; bound by `UMainScreenHUDComponent` to navigate back to index 0

**Key Methods:**
- `virtual void Init()` — empty default; override in subclasses that need setup logic

**NativeConstruct:** binds `BackButton` click → broadcasts `OnBackRequested`

> **Note:** `BackButton` must be `protected` (not `private`) in `UBaseScreen` — `BindWidget` requires the property to be visible to the engine's reflection system. Private breaks the binding silently.

---

#### UDragHandle / UResizeHandle
**Type:** `UUserWidget` | **Blueprints:** `WE_DragHandle`, `WE_ResizeHandle`

Hit-testable widgets that capture mouse and forward drag/resize events to their parent `UDraggablePanel` via `GET_OUTER(UDraggablePanel, Panel, FReply::Unhandled())`. Guard mouse move with a `bActive` flag — `NativeOnMouseMove` fires on hover too.

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

**Config:** `SpawnVolume` (set at runtime by `USessionHUDComponent`), `Impulse`, `AngularImpulse`, `ImpulseRange`, `AngularImpulseRange`, `TimeBeforeDestroyingDice` (5s)

**Roll Modes:** Normal / Advantage / Disadvantage — mode buttons enabled only when exactly one selector has `DiceCount == 1`. Advantage/Disadvantage spawn 2 dice and keep only the higher/lower result; losing die gets `bWasKept = false`.

**Delegates:**
- `OnAllDiceRolled` (`TArray<FRollResult>`, `EDiceRollMode`)
- `OnDiceFailsafeDestroyed` (`EDiceType`)
- `OnRollInitiated` (zero-param) — broadcast at the **very start** of `RollDice`, before spawning

> **Note:** `bRollInProgress` must be set `true` before the spawning loop, not after — otherwise `ResetCount` resets `RollMode` mid-roll.
>
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
>
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

#### ASessionPawn
**Type:** `APawn` | **Blueprint:** `P_Session`

Top-down camera rig.

**Component hierarchy:** `Root (USceneComponent) → Sphere (UStaticMeshComponent) → SpringArm → Camera`

All components are public so `ASessionController` can access `SpringArm->TargetArmLength`.

---

### PlayerControllers/

#### AMainScreenController
**Type:** `APlayerController` | **Blueprint:** `PC_MainScreen`

UI-only controller for the main screen. No camera pawn, no Enhanced Input.

**Constructor:** `CreateDefaultSubobject<UMainScreenHUDComponent>("HUDComponent")`

**BeginPlay:** `bShowMouseCursor = true`, `FInputModeUIOnly()`

---

#### ASessionController
**Type:** `APlayerController` | **Blueprint:** `PC_Session`

Central hub for all player input and HUD management.

**Constructor:** `CreateDefaultSubobject<USessionHUDComponent>("HUDComponent")`

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
>
> **Note:** `PostEditChangeProperty` is wrapped in `#if WITH_EDITOR` — for runtime validation use `ValidateCameraSettings()` directly.

---

### Utility/

#### UFunctionLibrary
**Type:** `UBlueprintFunctionLibrary`

General-purpose helper functions accessible from C++ and Blueprint.

**Functions:**
- `GetTypedWidgetFromName<T>(UUserWidget* Widget, FName Name)` → `T*` — template; casts result of `GetWidgetFromName`. Logs a warning if `Widget` is null or the cast fails. **C++-only** (no `UFUNCTION` — templates can't be `UFUNCTION`). Use this everywhere instead of `Cast<T>(Widget->GetWidgetFromName(...))`.
- `GetEnumDisplayName<T>(T Value)` → `FString` — template; returns the display name for a `UENUM` value via `StaticEnum<T>()`. Requires `DisplayName` metadata on each enum value. **C++-only**. Use instead of `GetValueAsString` + `RightChop` pattern.
- `GetSessionSaveSlotName(USessionInstance*)` → `FString` — returns `"Session_{SessionID}"` or empty string on null. All session save/load calls go through this; never hardcode the slot name.
- `LoadSessionSave(UObject* WorldContext)` → `USessionSave*` — gets `USessionInstance` from the world context, resolves the slot name, loads and returns the save object. Returns nullptr (with warning) on any failure. Use this everywhere instead of inline `GetGameInstance` + `LoadGameFromSlot` blocks.
- `GetLocalPlayerName(UObject* WorldContext)` → `FString` — returns the local player's name from `PlayerState->GetPlayerName()`, or `"Unknown"` on failure. Use this everywhere instead of inline `GetPlayerController(0)` + null-check chains.
- `MakeParticipantKey(TArray<FString> Participants)` → `FString` — sorts the participant list and joins with `|` to produce a stable channel identity key. Use everywhere a pipe-joined participant key is needed; never sort+join inline.

> **Note:** Template definitions must live entirely in the `.h` — no `.cpp` entry needed. Define inside the class body (implicit `inline`) or outside with `inline`.

---

#### MacroLibrary.h
**Type:** Plain header (no class, no `.cpp`)

Project-wide utility macros for common null-guard and outer-retrieval patterns. Include wherever needed.

**Macros:**
- `GET_OUTER(Type, VarName, ReturnVal)` — calls `GetTypedOuter<Type>()`, assigns to `VarName`, and returns `ReturnVal` with a warning log if the result is invalid. Used in `UDragHandle` and `UResizeHandle`.
- `GET_OWNING_PC(VarName, ReturnVal)` — casts `GetOwningPlayer()` to `APlayerController*`, assigns to `VarName`, and returns `ReturnVal` with a warning log if the cast or validity check fails. Used in `UChatBox`.
- `CHECK_IF_VALID(VarName, ReturnVal)` — checks `IsValid(VarName)` and returns `ReturnVal` with a warning log if the check fails. The single most common null-guard pattern — use everywhere instead of the three-line `if (!IsValid) { UE_LOG; return; }` block.

> **Note:** Macros are used here because they require injecting a local variable name and/or causing the caller to `return` — neither is possible with a template function. These are the only cases where a macro is preferred over a template.

---

#### UDelegateLibrary
**Type:** `UBlueprintFunctionLibrary`

Holds shared delegate type declarations used across multiple screens and systems. All cross-screen delegates are declared here to avoid name collisions and keep delegate types in one place.

**Declared types:**
- `FOnBackRequested` (`DECLARE_DYNAMIC_MULTICAST_DELEGATE`) — declared on `UBaseScreen`; broadcast by all screen subclasses when the user clicks Back; bound by `UMainScreenHUDComponent` to navigate to index 0

---

#### FContextMenuOption
**Type:** `USTRUCT`

Data for a single context menu item. Holds a `ButtonName` (`FString`) and `OnClicked` (`TDelegate<void()>`, non-dynamic — supports `BindLambda`).

---

#### UContextMenuButton
**Type:** `UUserWidget` | **Blueprint:** `WE_ContextMenuButton`

Single button row inside a `UContextMenu`. **Bound Widgets:** `MenuButton` (`UButton`), `ButtonLabel` (`UTextBlock`). `SetOption(FContextMenuOption)` sets the label text and binds the click delegate.

---

#### UContextMenu
**Type:** `UUserWidget` | **Blueprint:** `W_ContextMenu`

Generic floating context menu. Add to viewport via `AddToViewport()`, position with `SetMenuPosition`, auto-dismisses on any click outside the content box.

**Bound Widgets:** `ContextBox` (`USizeBox`), `MenuOptionsBox` (`UVerticalBox`)

**Config (EditAnywhere):** `ContextMenuButtonClass` (`TSubclassOf<UContextMenuButton>`)

**Key Methods:**
- `SetMenuOptions(TArray<FContextMenuOption>)` — clears existing buttons, spawns one `UContextMenuButton` per option. Wraps each option's `OnClicked` in a lambda that calls `CloseMenu()` before the original callback, so buttons also dismiss the menu.
- `SetMenuPosition(FVector2D)` — offsets `ContextBox` within the root `Overlay` using `UOverlaySlot::SetPadding`. Call after `AddToViewport()`.
- `CloseMenu()` — calls `RemoveFromParent()`.

`NativeOnMouseButtonDown` checks whether the click falls within `ContextBox`'s cached geometry bounds. If outside: calls `CloseMenu()` and returns `FReply::Handled()`. If inside: returns `FReply::Unhandled()` so child buttons receive the event.

> **Blueprint requirement:** The `W_ContextMenu` root must be an `Overlay` that fills the viewport. `ContextBox` and `MenuOptionsBox` should be set to `Not Hit Testable Self Only` so missed clicks pass through to the root and trigger dismissal.

---

### Environment/ *(planned — not yet implemented)*

#### AEnvironmentManager
**Type:** `AActor` (Replicated) | **Place in level:** yes (one instance)

Controls time of day and weather. `USessionHUDComponent` finds it at BeginPlay via `GetActorOfClass`.

**Replicated properties:** `CurrentTimeOfDay` (float, 0–24), `CurrentWeatherType` (`EWeatherType`), `WeatherIntensity` (float, 0–1) — each has an OnRep function that applies the change locally.

**Server RPCs:** `Server_SetTimeOfDay(float)`, `Server_SetWeather(EWeatherType, float)`

**Level refs (set in BP):** `ADirectionalLight` (sun), `ASkyLight`, `AExponentialHeightFog`, `AWindDirectionalSourceComponent`, `ANiagaraActor` per weather type

**Time of day:** Maps 0–24 to sun pitch/yaw on the `ADirectionalLight`. `USkyAtmosphereComponent` responds automatically.

**Weather:** Enables/disables the matching `ANiagaraActor`, adjusts `AExponentialHeightFog` density, and sets `AWindDirectionalSourceComponent` strength.

---

#### EWeatherType
**Type:** `UENUM` | Values: `Clear`, `Overcast`, `Rain`, `HeavyRain`, `Snow`, `Blizzard`, `Wind`, `Sandstorm`

---

#### UEnvironmentControlPanel
**Type:** `UUserWidget` | **Blueprint:** `WE_EnvironmentControlPanel`

GM panel for setting time of day and weather. Registered with `UTaskbar` as a `UDraggablePanel` via `USessionHUDComponent`.

**Controls:** Time slider (0–24, with formatted label e.g. `"14:30"`), weather type button row (one per `EWeatherType`), intensity slider (0–1, disabled when weather = Clear).

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
- **Current registered folders:** `AssetLibrary`, `CampaignBrowser`, `CampaignManager`, `Chat`, `Components`, `Dice`, `GameModes`, `GameStates`, `PlayerControllers`, `PlayerList`, `PlayerStates`, `Pawns`, `SaveLoad`, `Settings`, `UI`, `Utility`
- **Pending (Environment system):** Add `Environment` to `PublicIncludePaths` when the `Environment/` source folder is created

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

### Delegates
- **`DECLARE_DYNAMIC_MULTICAST_DELEGATE` names are global** — two headers declaring the same delegate name cause a compile error. Declare shared delegate types once in `UDelegateLibrary` (`Utility/DelegateLibrary.h`) and include that header wherever the type is needed. Per-class delegates (e.g. `FOnGameTypeSelected`) stay in their own class header.

### C++ Patterns
- Declaring `Type* MemberName = Cast<...>` in `BeginPlay` creates a local variable that shadows the member — use `MemberName = Cast<...>` (no type prefix)
- `FRotator(Pitch, Yaw, Roll)` — constructor order is **not** (X, Y, Z); wrong order causes violent camera bouncing
- `GetActorForwardVector()` has a Z component when pitched — zero out `Delta.Z` after computing movement to keep it flat
- `FMath::Sign(float)` returns 1.f / -1.f / 0.f — good for collapsing scroll direction checks
- `UEnum::GetValueAsString()` returns `"EnumClass::ValueName"` — use `UFunctionLibrary::GetEnumDisplayName<T>(Value)` instead; requires `UMETA(DisplayName = "...")` on each enum value
- `UPhysicalMaterial` requires the `PhysicsCore` module in `Build.cs`
- A `USTRUCT` or `UCLASS` member that holds raw `UObject*` pointers (including `TArray<UObject*>`) must use `UPROPERTY()` — without it the GC can't track the references and they may be prematurely collected
- `USlider::SetValue` fires `OnValueChanged` the same as a user drag — guard with `ClampedValue != Value` to prevent recursion
- **`BindWidget` in a base `UUserWidget` class must be `protected`, not `private`** — private `BindWidget` members in a base class silently break widget binding in all subclasses; use `protected` so the property is visible to UE's reflection system
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
- [x] `ASessionController` — all input, HUD component, camera movement
- [x] `ASessionPawn` — top-down camera rig
- [x] Camera movement — pan, zoom, sprint, pan reset, editor property validation
- [x] `AMainScreenController` + `UMainScreenHUDComponent` — main screen flow
- [x] `S_MainScreen` — `WidgetSwitcher` root; `S_HomeScreen` index 0, `S_SettingsScreen` index 1
- [x] **Runtime camera settings menu** (`USaveGame`-based)
  - [x] `UCameraSettingsSave` — save class with all 9 camera fields
  - [x] `ASessionController` save/load/validate — `ValidateCameraSettings`, `ApplyCameraSettings`, `SaveCameraSettings`; loads on `BeginPlay`
  - [x] `USettingsSlider` — reusable slider+text widget with paired min/max clamping
  - [x] `WE_SettingsSlider` — Blueprint widget layout
  - [x] `S_SettingsScreen` — 9 sliders wired and grouped; Apply, Reset, Back buttons added
  - [x] `UMainScreenHUDComponent` settings wiring — refactored; settings logic moved to `UCameraSettingsPanel`; home screen logic moved to `UHomeScreen`; component now handles screen-level navigation only
- [x] Private messaging — `@Name` syntax, per-conversation tabs, server-side routing
- [x] Panel layout persistence — `UPanelLayoutSave`; saved on drag/resize/toggle; restored on startup
- [x] Taskbar minimize system — `UTaskbar` + `UTaskbarButton`
- [x] Draggable and resizable panels — `UDraggablePanel`, `UDragHandle`, `UResizeHandle`
- [x] Close and reopen private chat tabs
- [x] Home screen → Campaign Manager navigation (Play button replaced with Campaign Manager button; CampaignBrowser and AssetLibrary stub screens added)
- [~] Session management (start, load, save) — `USessionInstance`, `USessionSave`, `UPlayerSave`, `ASessionGameState`, `ASessionPlayerState`, `ASessionGameMode` all implemented; `InitGame`/`PostLogin`/`Logout` logic complete; login options flow implemented (`?PlayerID=<guid>` passed via travel URL, parsed in `PostLogin`); remaining gap: `UCampaignManagerScreen::OnCampaignSelected` `ServerTravel` call needs to be routed through a server RPC
- [ ] Session save/load — full snapshot (map, tokens, fog of war, initiative, chat, sheets, notes, inventory); sessions stored as `"Session_{SessionID}"` slots indexed by `UCampaignManagerSave`; manual save + autosave (configurable interval); auto-save on session close
- [ ] GM permissions system
- [ ] Session player cap (default 8, removable)
- [x] Tab renaming (client-local) — right-click opens `UContextMenu` with Rename and Close options; rename persisted to `USessionSave::ChatTabNames`; close button removed from tab in favour of context menu
- [x] Chat log persistence
- [ ] Shared notes (rich-text: headers, bullets, bold, italic; real-time collaborative editing; persists across sessions; accessible outside active session via Campaign Manager)
- [ ] Pre-session lobby (waiting room; pre-game chat; character sheet accessible while waiting; Host sees connection status and launches when ready)
- [ ] Session discovery and join flow
  - [ ] Invite code — immediate join, no approval
  - [ ] Public browser — sends join request; requester gets temporary chat access to introduce themselves; Host approves or declines
- [ ] Host disconnect handling (lock policy for actions when no Host is present; full policy TBD)

---

### Phase 3 — Campaign Manager

The Campaign Manager is the primary hub between the home screen and an active session. Accessed via the Play button on the home screen.

- [x] `UCampaignManagerSave` — save game; `TMap<FString, FCampaignList>` keyed by game type; slot `"CampaignManager"`
- [x] `UGameTypeButton` — tab button per game type; greyed out if no campaigns exist
- [x] `UCampaignCard` — campaign entry card; displays title, last played, player count; fires `OnCampaignSelected`
- [~] `UCampaignManagerScreen` — tab switching, selected state highlighting, and fake-data test mode complete; `OnCampaignSelected` is still a placeholder; Blueprint wiring pending
- [ ] Campaign Manager screen — lists all campaigns the player is part of (full flow pending)
- [ ] Campaign creation — any player can create; private (invite-only) or public (discoverable)
- [ ] Public campaign browser — filterable by name, game system, tags, one-shot vs. multi-session, meeting days/frequency/session length
- [ ] Direct invite link / invite code support
- [ ] Campaign card — per-game-system content (D&D 5e: character sheet, notes, party list, character art, next session)
- [ ] Player Profiles — username (required), bio (optional), games played (auto from campaign history + self-reported); shown alongside join requests
- [ ] Campaign locking to a single game system at creation
- [ ] Scheduling — meeting days, frequency, typical session length; surfaces on campaign card and public browser filter
- [ ] Notes accessible outside active session (read and edit via Campaign Manager)

---

### Phase 4 — Map Builder

**Planned C++ Classes:**
- `ATileActor` — base tile actor; snaps to square grid; stores grid coordinate (X, Y, HeightLevel); supports variable sizes (FIntPoint footprint from `UTileData`)
- `UTileData` — UPrimaryDataAsset; per-tile config: mesh, material, passable flag, elevation offset, size
- `APropActor` — base prop actor; surface-snaps to tile beneath (arbitrary X/Y); free rotation and scale
- `UPropData` — UPrimaryDataAsset; per-prop config: mesh, scale range, snap behavior
- `AMapGrid` — defines playable area and tile size; handles world-space ↔ grid coordinate conversion; owns ghost/preview tile for placement
- `AMapBuilderController` — manages build mode vs play mode, ghost preview, placement/rotate/delete logic
- `UMapSave` — USaveGame; serializes all placed tile/prop data (UUID, grid position, rotation, scale); map metadata (name, type, thumbnail)
- `UMapBuilderHUDComponent` — UActorComponent; manages builder UI lifecycle
- `UTileBrowser` — UUserWidget; category tabs, thumbnail grid, selected item preview
- `UMapLocationPin` — world map prop subtype; stores a combat map ID for drill-down linking

**Build order within this phase:**
1. Core flat tile/prop placement — grid, placement, save/load, built-in assets only
2. Height system — 3D grid coordinates, ramps, multi-floor
3. Scale modes — Combat Map vs World/Region Map as distinct types
4. Session/Host integration — bring map into live session, Host-only gate

**Checklist:**
- [ ] `ATileActor` + `UTileData` — tile placement, grid snapping, variable sizes
- [ ] `APropActor` + `UPropData` — free-floating prop placement, surface snapping
- [ ] `AMapGrid` — grid definition, coordinate conversion, ghost preview
- [ ] `AMapBuilderController` — build mode / play mode, placement input
- [ ] `UMapSave` — map serialization and load
- [ ] `UTileBrowser` — tile/prop browser UI
- [ ] `UMapBuilderHUDComponent` — builder HUD lifecycle
- [ ] Height system — 3D tile coordinates, elevation increments, ramps
- [ ] Combat Map and World/Region Map scale modes
- [ ] `UMapLocationPin` — location pins with combat map references
- [ ] World → combat map drill-down via location pins
- [ ] Grid overlay toggle (square; hex planned for later)
- [ ] Undo/redo
- [ ] Flat image map support (secondary to tile builder)
- [ ] Fog of war (brush and region/tile selection tools; re-fog at any time; GM-controlled opacity: opaque/pitch black/semi-transparent)
- [ ] Auto-reveal — map reveals within player token line of sight as tokens move; GM toggle per session
- [ ] Camera boundary — fog of war and GM-defined barriers act as hard camera limits for players
- [ ] Entity-based vision system
- [ ] Hex grid (after square system is stable)

---

### Phase 5 — Environment System (Time of Day & Weather)

- [ ] `EWeatherType` enum (Environment/) — Clear, Overcast, Rain, HeavyRain, Snow, Blizzard, Wind, Sandstorm
- [ ] `AEnvironmentManager` — replicated level actor; controls `ADirectionalLight` (sun angle → Sky Atmosphere), `ASkyLight`, `AExponentialHeightFog`, `AWindDirectionalSourceComponent`, and `ANiagaraActor`s per weather type. Server RPCs: `Server_SetTimeOfDay(float)`, `Server_SetWeather(EWeatherType, float Intensity)`. Replicated properties with OnRep apply logic.
- [ ] `UEnvironmentControlPanel` — `UUserWidget`; time slider (0–24 + formatted label), weather type button row, intensity slider. Calls RPCs on `AEnvironmentManager`. Registered with `Taskbar` as a `UDraggablePanel` via `USessionHUDComponent`.
- [ ] Wire into `USessionHUDComponent` — find `AEnvironmentManager` at BeginPlay via `GetActorOfClass`; call `FindAndRegisterPanel` for EnvironmentPanel
- [ ] Blueprint side — place `AEnvironmentManager` in `L_Session`, wire light/fog/Niagara refs in BP, build `WE_EnvironmentControlPanel`

---

### Phase 6 — Miniatures

- [ ] Base miniature actor/pawn
- [ ] Grid placement and snap
- [ ] Freeform movement within movement radius (range enforced; range shrinks as movement is spent)
- [ ] Difficult terrain (double movement cost; GM toggle per session — enforced or honor system)
- [ ] Diagonal movement setting (GM-controlled: D&D 5e style / Pathfinder style / others TBD)
- [ ] Scale system (combat vs. non-combat maps; combat maps follow game system unit rules)
- [ ] Default miniature — wooden artist's mannequin, natural wood tone, animated (walk cycle + idle)
- [ ] Mini labels (name, HP, conditions)
- [ ] Entity Management Panel — centralized GM panel for NPCs, player-summoned creatures, and controllable items; tracks HP, conditions, and turn order across multiple entities; players access it for their own summoned creatures
- [ ] Vision system — player vision vs. character vision separation; outline rendering for characters inside vision-blocking effects
- [ ] User-importable meshes (glTF with Draco compression; deferred until mesh import phase)

---

### Phase 7 — D&D 5e / 2024 Game System

- [ ] Game system plugin architecture (core layer + game system plugin; adding a new ruleset = new plugin, no core changes)
- [ ] Character creation — root layer (name, appearance, bio, portrait) + game system layer (stats, class, race, skills, etc.); supports custom races/classes/backgrounds
- [ ] Character sheet (STR/DEX/CON/INT/WIS/CHA, proficiency bonus, saving throws, skill modifiers, HP, hit dice, spell slots, conditions, inventory)
- [ ] Character sheet visibility (owner + GM always; owner can share with other players)
- [ ] Initiative tracker — public turn order; GM staging list (hidden NPCs/reinforcements); round tracking; manual turn advancement; skip/delay; remove combatants
- [ ] Initiative rolling (manual via dice UI and automatic from character stats)
- [ ] Combat flow — GM-driven start and end; action economy (action, bonus action, reaction, movement)
- [ ] Spell management (choose spells, track slots, concentration, components, spell effects)
- [ ] Condition tracking (auto-apply from game events; GM toggle for full manual control per table)
- [ ] Attack rolls, saving throws, skill checks tied to stats
- [ ] Monster/NPC stat blocks
- [ ] Inventory and loot — item transfer ("Send to Player" with quantity prompt for stacks); GM master item list; custom item creation (root layer + game system layer)
- [ ] XP award (GM grants XP) and level-up signaling (GM signals; player handles leveling flow: features, spells, HP)
- [ ] Measurement tools — ruler and AoE templates (cone, sphere, line, cube); universal core tools, first implemented here
- [ ] Rule variants per campaign (diagonal movement ruling, optional rules, house rules)

---

### Phase 8 — UI & Polish

- [ ] UI theming system — decoupled from game system; per-player customization
- [ ] Panel notification system (badge count, flash, or icon on collapsed/hidden panels with new activity)
- [ ] Replace default Unreal UI assets with custom art
- [ ] Sound system
  - [ ] Music — background tracks heard by all players simultaneously
  - [ ] Ambient audio — environmental loops heard by all players simultaneously
  - [ ] Sound effects — proximity-based one-shot/looping SFX; player hears only if their mini is close enough to the source
  - [ ] Map-baked audio — default music/ambient tracks assigned during map build; play on map load; GM can toggle
  - [ ] User-uploadable audio (WAV)
- [ ] Visual effects system

---

### Phase 9 — Custom Content & Extensibility

- [ ] `UAssetImporter` — generic importer (extensions, destination, dialog, copy, delegate)
- [ ] Asset Library screen — file explorer UI; folder-based; import button + drag-and-drop
- [ ] Image import (PNG/JPG → Maps)
- [ ] Audio import (WAV → Sounds)
- [ ] Mesh import — deferred; requires runtime loader plugin
- [ ] Custom dice import
- [ ] Additional TTRPG systems beyond D&D 5e
- [ ] Player-to-player asset sharing (opt-in; asset owner approves before transfer)
- [ ] Cloud asset storage (future)

---

### Multiplayer Architecture

**Decided:**
- **Server Owner** and **Host** are separate roles. Server Owner = technical admin (create/close session, kick, transfer ownership). Host = game facilitator (bring maps into play, manage game state, advance turns). Server Owner assigns the Host role to any player.
- The Host role is system-level and game-agnostic — games with a traditional GM use it as GM; GM-less games assign it to whoever is facilitating.
- Host can delegate specific powers to individual players (e.g., move enemy tokens, bring in a map). Specific delegatable actions TBD when built.
- Default player cap: 8 (Host included); removable; no hard engine limit
- When no Host is present, certain actions lock (moving NPCs, editing stats); full lock policy TBD when session management is built
- **Mesh distribution:** when Host brings a map into a session, assets missing from client machines auto-transfer from Host before the map renders. No placeholders — all clients see the map in full. Assets cached locally after first transfer.
- **Server model:** Listen server — the Server Owner hosts the session from their own machine. Acceptable tradeoff at this scale (2–8 players, non-persistent sessions). Code must be written to make a future switch to dedicated server seamless: all authoritative state lives on the server, clients never trust themselves, and the Server Owner role is checked via a flag (not `IsLocalController()` or similar) so swapping to a dedicated server changes who holds the flag, not the logic.
- **GM role supports multiple players simultaneously** — a session can have more than one GM active at the same time.
- **GM role is transferable** — any existing GM or the Server Owner can promote or demote players at any time.
- **Default GM on session creation:** the campaign creator. Stored in `USessionSave` as `TArray<FGuid> GMPlayerIDs`.
- **`USessionSave` runtime role fields:** `HostPlayerID` (`FGuid`, Server Owner), `GMPlayerIDs` (`TArray<FGuid>`), `PlayerIDs` (`TArray<FGuid>`)
- **Session discovery:** Public browser (filterable by name, game system, tags, schedule, etc.) + direct invite link/code.
- **Join flow:** Invite code = immediate join, no approval. Public browser = join request; requester gets temporary chat access to introduce themselves; Host approves or declines.
- **Pre-session lobby:** Waiting room before session starts. Shows who is connected, has pre-game chat, lets players access character sheets. Host sees connection status and launches when ready.

**Session data ownership model:**

| Data | Owner | Storage | Notes |
|---|---|---|---|
| Session ID, display name, player cap, game type | Server (Host machine) | `USaveGame` on Host machine + `GameState` at runtime | Save is source of truth; `GameState` is runtime copy |
| Approved player list | Server (Host machine) | `USaveGame` on Host machine + `GameState` at runtime | Editable in and out of session; in-session changes write back to save |
| Map state | Server (Host machine) | `USaveGame` on Host machine + `GameState` at runtime | Grows as map builder is built |
| Combat / initiative state | Server (Host machine) | `USaveGame` on Host machine + `GameState` at runtime | Restored on reload if session closed mid-combat |
| Chat log | Server (Host machine) | `USaveGame` on Host machine | Persists across sessions |
| Volatile copies of player data (character sheets, etc.) | Server (runtime only) | `PlayerState` | Populated on join, discarded on disconnect/session end. Never written to disk by the server. |
| Character sheets, private notes | Player | `USaveGame` on player's machine | Server requests a copy on join; player machine is always the source of truth |

**Session lifecycle:**

**Server startup:**
1. Host launches the session from the Campaign Manager
2. Server starts and loads `USessionSave` — `GameState` is populated with last saved state (map, chat log, approved player list, combat state, etc.)
3. Host controls activate
4. Players already running the game receive an in-app notification that the session is live (approved players not yet connected only; delivery to offline players deferred to notification system)

**Player join (before session starts):**
1. Player opens the Campaign Manager and finds the campaign
2. If the Host is present — join button drops them straight into the session flow
3. If the Host is absent — join button drops them into the lobby
4. Server checks the joining player against the approved player list in `GameState`
5. If approved — player enters. If not — rejected. Host presence is not required for this check.
6. In the lobby, players can chat with each other and access any player-owned data that doesn't require the Host (character sheets, personal notes, mini selection)

**Session start:**
1. Host arrives (if not already present)
2. Server requests fresh player data from all connected clients
3. Each client sends their current `UPlayerSave` data; server populates `PlayerState` for each player — player machine is always source of truth, overwriting any cached copy
4. Host triggers session start
5. Map loads and full session state is restored from `USessionSave`

**Late join (session already active):**
- Same flow as session start join, lobby step skipped
- Server requests fresh data from the late joiner individually and populates their `PlayerState`
- Player is dropped directly into the active session
- "Request fresh player data" logic is shared between session start and late join — one function, two call sites

**Migration note:** All server-held save data is treated conceptually as belonging to "the server" even though it physically lives on the Host's machine in the listen server model. This data must be clearly grouped and labeled so that migrating to a dedicated server later is a matter of moving where that data is stored — not rearchitecting how it flows.

**Save file inventory:**

| Class | Owner | Scope | Contents |
|---|---|---|---|
| `UCampaignManagerSave` | Any player | Global | Campaign records — already exists |
| `USessionSave` | Server (Host machine) | Per session | Session metadata, map state, chat log, approved player list, combat state. Linked to campaign via `CampaignID (FGuid)`. Has own `SessionID (FGuid)`. |
| `UGMSave` | GM | Per campaign | NPC notes, encounter prep, loot tables, location notes, permissions config. Campaign-scoped by default. Global GM library (opt-in cross-campaign sharing) deferred to a later phase. |
| `UPlayerSave` | Player | Per campaign | Character sheet, private notes. Source of truth — server never writes to this. |
| `UPanelLayoutSave` | Player | Global | Panel positions/sizes/visibility — already exists, stays separate |
| `UCameraSettingsSave` | Player | Global | Camera settings — already exists, stays separate |

**Save slot model:**

Sessions are stored using Unreal's built-in save slot system — no custom file I/O. The slot name encodes the session identity: `"Session_{SessionID}"`. `UCampaignManagerSave` is the authoritative index: each `FCampaignRecord` contains a `TArray<FGuid> SessionIDs` listing every session that belongs to that campaign. To find all sessions for a campaign, look up its record in the index. To load the most recent session, sort by timestamp stored in `USessionSave` and load by slot name. To open a campaign = find its `FCampaignRecord`, read `SessionIDs`, load the most recent (or let GM pick from the list).

This approach keeps all save I/O within UE's native save game system and makes a future dedicated-server migration straightforward — the index and session slots move to wherever the server runs, no path logic to change.

---

*Last updated: 2026-04-15* — Code quality pass: `MacroLibrary.h` added (`GET_OUTER`, `GET_OWNING_PC`, `CHECK_IF_VALID`); `UFunctionLibrary` gains `MakeParticipantKey` and `GetEnumDisplayName<T>`, removes `GetDiceName`; `UChatBox` gains `ParseMentions` private helper; `FindOrCreateChannel` optimized via `MakeParticipantKey`; `USessionHUDComponent` gains `Panels` array (loop replaces triple-call save/load); `CHECK_IF_VALID` applied across ~11 files (~33 sites); `GET_OUTER` applied to `UDragHandle`/`UResizeHandle`; `GET_OWNING_PC` applied in `UChatBox`. (~176 lines removed.)

*Last updated: 2026-04-14 (updated)* — `UFunctionLibrary` gains three static helpers extracted from repeated inline patterns: `GetSessionSaveSlotName(USessionInstance*)`, `LoadSessionSave(UObject*)`, `GetLocalPlayerName(UObject*)`. All call sites in `UChatBox` and `USessionHUDComponent` updated to use these. `USessionSave` gains `ChatTabNames` (`TMap<FString, FString>`) for persisting user-assigned tab labels. `UChatBox` Config gains `ContextMenuClass`. (~50 lines of inline boilerplate removed across 4 call sites.)

*2026-04-14* — Chat tab rename complete. `UChatBox` wired: `OnTabRightClickedHandler` spawns `W_ContextMenu` at cursor with Rename and Close options; `OnTabRenamedHandler` persists new name to `USessionSave::ChatTabNames`. Close button removed from `UChatTab` — channel close now lives in the context menu alongside rename. `UChatTab` updated: `CloseButton` BindWidget, `FOnTabClosed` delegate, and `SetCloseable` removed; `EditLabel` type corrected to `UEditableText` (was `UEditableTextBox`); `GetChannel()` accessor added. `UContextMenu` updated: bounds-check approach in `NativeOnMouseButtonDown` replaces backdrop button pattern; `ContextBox` (`USizeBox`) added as BindWidget; `SetMenuPosition(FVector2D)` added (offsets `ContextBox` via `UOverlaySlot`); Blueprint renamed `W_ContextMenu`. `UContextMenuButton` Bound Widgets corrected: `MenuButton` + `ButtonLabel`.

*2026-04-13* — Chat tab rename infrastructure complete. `UChatTab` gains `EditLabel` (`UEditableTextBox` BindWidget), `EnterRenameMode()`, `OnTabRightClicked` and `OnTabRenamed` delegates. `UContextMenu` and `UContextMenuButton` added to `Utility/` — generic floating context menu with auto-dismiss on click-outside and on button click (lambda wrapping in `SetMenuOptions`). `UContextMenu::NativeOnMouseButtonDown` consumes all clicks reaching the root without `Super::`. `UChatBox` wiring (Step 3) is the only remaining rename step. Format-all pass complete across all 90+ source files. 15 recurring code patterns catalogued in `memory/project_pattern_analysis.md`.

*Last updated: 2026-04-12 (updated)* — Chat log persistence implemented. `USessionSave` gains `FChatMessageRecord`, `FChatLogRecord`, and `ChatLog` (`TMap<FString, FChatLogRecord>`). `UChatChannel` gains `RestoreMessage`. `UChatBox` gains `FindOrCreateChannel` (extracted from `AddChatMessage`; shared with restore loop). `USessionHUDComponent::BeginPlay` restores chat log on load; `SendChatMessageOnServer` saves each message after routing. `bPendingRefocus` flag added to `UChatBox` to fix Enter double-fire bug (Slate fires `OnUserMovedFocus` immediately after `OnEnter`). Roadmap: chat log persistence checked off.

*Last updated: 2026-04-11* — `USessionInstance` added (`GameInstances/`). `ASessionGameMode` fully implemented: `InitGame` loads `USessionSave` into `ASessionGameState`; `PostLogin` assigns role flags and adds player to correct list; `Logout` removes player from list. `ASessionPlayerState` gained `SessionPlayerID` (Replicated FGuid) with full accessors. `ASessionGameState` gained full getter/setter API for all fields. `GameInstances/` folder added to source tree and Build.cs. Known gap documented: `SessionPlayerID` not yet set via login options.

*2026-04-10 (updated 2)* — Folder structure corrected: `GameModes/`, `GameStates/`, `PlayerStates/` added to source tree overview; `SessionSave` added to SaveLoad line.

*2026-04-10 (updated 1)* — `USessionSave` implemented (SaveLoad/). `ASessionGameState` and `ASessionPlayerState` implemented with replication. `ASessionGameMode` stub added (GameModes/). `GM_Session` updated: Game State Class → `ASessionGameState`, Player State Class → `ASessionPlayerState`. Build.cs updated: `GameStates`, `PlayerStates`, `GameModes` added to `PublicIncludePaths`. Roadmap session management item marked in-progress.

*2026-04-10 (updated)* — GM role finalized: multiple GMs supported per session, role is transferable, default GM = campaign creator. `USessionSave` role fields added: `HostPlayerID`, `GMPlayerIDs`, `PlayerIDs`. Save disk layout replaced with slot-name model: sessions stored as `"Session_{SessionID}"` save slots; `UCampaignManagerSave` is the authoritative campaign→session index via `FCampaignRecord.SessionIDs`. No custom file I/O.

*2026-04-10* — Session lifecycle fully designed: server startup, player join, lobby, session start, late join. "Request fresh player data" identified as shared logic (one function, two call sites). Save file inventory defined: `USessionSave`, `UGMSave`, `UPlayerSave`; global GM library deferred. Disk layout documented. Session data ownership model and migration note added. Server model confirmed as listen server.

*2026-04-09 (updated)* — `UBaseScreen` added as shared base for all main screen widgets (`BackButton`, `OnBackRequested`, `virtual Init()`). `UCampaignManagerScreen`, `USettingsScreen` refactored to inherit `UBaseScreen`. `UAssetLibraryScreen` and `UCampaignBrowserScreen` added as stubs (`AssetLibrary/`, `CampaignBrowser/` source folders). `UHomeScreen` updated: Play button replaced with Campaign Manager button; CampaignBrowser and AssetLibrary buttons added; all four navigation delegates now wired through `UMainScreenHUDComponent`. `UMainScreenHUDComponent` now manages five screens (indices 0–4). `UCampaignManagerScreen::BuildFakeData` expanded to 9 game types with 20 DnD campaigns. `BindWidget` in base class needs `protected` gotcha added. Phase 2 roadmap item checked off.

*2026-04-09* — `UDelegateLibrary` added to `Utility/` for shared delegate declarations. `FOnBackRequested` declared there; `USettingsScreen` and `UCampaignManagerScreen` both use it (replacing their individual delegate names). `UGameTypeButton` expanded: `SetTabColors`, `SetSelected`, `GetLabel`. `UCampaignManagerScreen` expanded: `SelectedTabColor`/`UnselectedTabColor`/`bUseFakeData` config, `ActiveButtons` state, `SetSelectedGameButton`, `BuildFakeData`. `UMainScreenHUDComponent` null-checks `MainScreenRef` after `CreateWidget`. Delegate naming gotcha updated to reference `UDelegateLibrary`.

*2026-04-08 (updated)* — Campaign Manager classes added: `UCampaignManagerSave`, `UGameTypeButton`, `UCampaignCard`, `UCampaignManagerScreen` (in progress). `CampaignManager/` source folder added. Coding standards expanded (include order, GC safety, no debug output in committed code). Delegate naming collision gotcha added. Phase 3 roadmap updated.

*2026-04-08* — Environment system designed (not yet implemented): `AEnvironmentManager`, `EWeatherType`, `UEnvironmentControlPanel` added as planned classes. Phase 2 roadmap updated with full environment system checklist. Phase 4 lighting item cross-references Phase 2. Build.cs note added for pending `Environment/` folder.

*2026-04-06* — Settings system refactored into `UCameraSettingsPanel`, `USettingsScreen`, `UHomeScreen`; `UMainScreenHUDComponent` now only handles screen-level navigation. `Settings/` source folder added.

*2026-04-03 (updated)* — Roadmap expanded to 8 phases to reflect full GDD scope. New Phase 3 (Campaign Manager) added: campaign creation, public browser, player profiles, scheduling, campaign cards. Phase 2 expanded: session save/load, shared notes, pre-session lobby, join flow, host disconnect handling. Phase 4 (Map Builder, was Phase 3): fog of war, auto-reveal, and camera boundary details added. Phase 5 (Miniatures, was Phase 4): freeform movement with range enforcement, difficult terrain, diagonal movement setting, entity management panel, and vision system added. Phase 6 (D&D 5e, was Phase 5): character creation, inventory/loot, XP/level-up, measurement tools, rule variants, and plugin architecture added. Phase 7 (UI & Polish, was Phase 6): sound system fully broken out (proximity SFX, map-baked audio, user audio). Multiplayer Architecture updated: server model, join flow, and pre-session lobby moved from "needs research" to "decided."

*2026-04-03* — Phase 3 roadmap expanded: full map builder planned with 10 new C++ classes, height system, scale modes (Combat/World), location pin linking, and build order. Multiplayer architecture updated: Server Owner and Host are now distinct roles; mesh auto-distribution policy documented.

*2026-04-02 (updated)* — `UMainScreenHUDComponent` settings fully complete: `OnApplyClicked` implemented, BeginPlay loads saved values and handles first-launch defaults. `UDraggablePanel` gained `DefaultPosition`/`DefaultSize` (EditAnywhere) and `ResetToDefaultLayout()`. `UTaskbar` gained `ResetButton` + `ResetLayout()` — resets all registered panels to their Blueprint-configured defaults without affecting visibility.

*2026-04-02* — `UFunctionLibrary::GetTypedWidgetFromName<T>` added (replaces all `Cast<T>(Widget->GetWidgetFromName(...))` calls). `UMainScreenHUDComponent` settings wiring mostly complete: all refs cached, `OnSettingsClicked`/`OnBackClicked`/`OnResetClicked` wired, `SettingsSliders` TArray added. `OnApplyClicked` and slider init from saved values still pending.

*2026-04-01 (updated 2)* — Camera settings system partially implemented: `UCameraSettingsSave`, `ASessionController` save/load/validate, `USettingsSlider` widget class. `S_Settings` Blueprint and `UHomeScreenHUDComponent` settings integration are next. Two new gotchas added (`BindWidget` type and slider recursion guard).

*2026-04-01 (updated 1)* — Home screen implemented: `AHomeScreenController`, `UHomeScreenHUDComponent`, `GM_HomeScreen`, `PC_HomeScreen`, `S_HomeScreen`, `L_HomeScreen`. Scene/controller management complete. Level naming convention added (L_ = final, Dev_ = dev/test). `USessionHUDComponent` input mode and cursor now owned by `ASessionController`.

*2026-04-01 (updated)* — Dice collision SFX implemented. Added home screen and scene/controller management to Phase 2. Fleshed out Phase 7 with `UAssetImporter` architecture, asset library screen design, import flow (type selector → dialog or drag-and-drop), and supported formats (PNG/JPG, WAV; mesh deferred).

*2026-04-01* — Added private dice roll initiation: `UChatBox::TrySendPrivateRollMessage`, `UDiceSelectorManager::OnRollInitiated` delegate, `USessionHUDComponent::OnRollInitiated` handler. Fixed `ExitChat` to preserve input field on click-away. `SwitchToChannel` now clears input on channel switch. Fixed `ReopenChannel` missing `UFUNCTION()`. Added two new gotchas. Earlier this session: added `SaveLoad/` folder, `UPanelLayoutSave`/`FPanelLayoutData`, panel layout persistence across all three panels.

*2026-03-31 (updated)* — Added `UChatChannelListEntry`. Updated `UChatTab` with close button, `OnTabClosed` delegate, and `SetCloseable`. Updated `UChatBox` with close/reopen channel logic, closed channel list panel (`ClosedChannelContainer`, `ChannelListButton`), `ClosedChannels` set, `CloseChannel`, `ReopenChannel`, `RefreshChannelList`, `OnChannelListButtonClicked`, and auto-reopen on incoming message. Checked off close/reopen tabs roadmap item. Added tab renaming to Phase 2 roadmap; noted `UChatChannel::DisplayName` is safe to rename per-user since routing uses `Participants`. Added `UButton` style padding gotcha.

*2026-03-31* — Added `UDraggablePanel`, `UDragHandle`, `UResizeHandle` classes. Updated `USessionHUDComponent` to document input mode initialization, `FindAndRegisterPanel` helper, and panel refs (`ChatPanel`, `DicePanel`, `PlayersPanel`). Updated `UChatBox` for private channel auto-routing, `NativeOnMouseButtonDown` passthrough fix, `SwitchToChannel` tab interactability, and cursor restoration in `ExitChat`. Updated `UChatTab` to document `SetInteractable`. Added `UI/Utility/` to content structure. Checked off draggable/resizable roadmap item. Added 10 new gotchas covering panel dragging, canvas slot anchoring, input mode setup, UMG fill sizing, taskbar panel registration, and private channel routing.

*2026-03-29* — Added `UTaskbarButton` and `UTaskbar` classes. Updated `USessionHUDComponent` to document `TaskbarRef` and `RegisterWidget` calls for Chat, Dice, and Players. Added taskbar minimize sub-item to Phase 6 roadmap. Added two new gotchas: taskbar toggle visibility check and Blueprint widget variant placement.

*2026-03-27 (updated)* — Participants bug fixed in `SendChatMessageOnServer`: now builds a full `Participants` list (sender + all recipients) before calling `AddChatMessageOnOwningClient`, so channels on recipients' clients include all parties. `UChatBox::GetActiveChannelParticipants()` added. `AddRollResultToChat` updated to route dice rolls to the active channel via `GetActiveChannelParticipants()`. New gotcha added: `Participants` must include the sender. All 30 source files fully documented with summary comments; headers reorganized into logical groups.

*2026-03-27* — Private messaging + tabbed chat + player list complete. `UChatBox` refactored to full tabbed container with `@`-syntax parsing. `USessionHUDComponent` `SendChatMessageOnServer` / `AddChatMessageOnOwningClient` updated with `Recipients` routing. `UPlayerRow` and `UPlayerList` added (`PlayerList/` folder). `USessionHUDComponent` binds `PlayerList->OnAddressClicked` → `ChatBoxRef->AppendToInput`. New gotchas added: `SwitchToChannel` UFUNCTION requirement, `PopulateList` on toggle, channel `Num()` guard.

*2026-03-26 (updated 1)* — UChatBox refactor complete. UChatBox is now a full tabbed chat container with `CreateChannel`, `SwitchToChannel`, and `AddChatMessage(Message, Participants, bIsSender)`. Scroll logic moved from UChatBox into UChatChannel (`Scroll(bool bUp)`, `ScrollMultiplier`). Next: update USessionHUDComponent to add recipients parameter to `AddChatMessageOnOwningClient`.

*2026-03-26 (updated)* — Chat folder created. Moved `UChatBox` and `UChatEntry` from `UI/` to new `Chat/` folder; added `Chat/` to `Build.cs`. Added `UChatChannel` and `UChatTab` classes (Chat/) as part of the tabbed private messaging system in progress. Phase 1 roadmap reordered: private rolls → sound → visuals → custom dice → physics tuning.

*2026-03-26* — Dice spawn volume implemented. Added `ADiceSpawnVolume` class (Dice/). Updated `UDiceSelectorManager` (replaced `StartingLocation` with `SpawnVolume`, dice now spawn at random points within the volume via `FMath::RandPointInBox`, null guard on `SpawnVolume`). Updated `USessionHUDComponent` (finds `ADiceSpawnVolume` in level via `GetActorOfClass` and assigns to manager in `BeginPlay`). Added two new gotchas: spawn volume must be in the level, and widget properties referencing level actors must be assigned at runtime.

*2026-03-25* — Advantage/disadvantage roll mode implemented and checked off. Updated `ABaseDiceActor` (bWasKept, FRollResult.DiceActor), `UDiceSelectorManager` (EDiceRollMode, three mode buttons, advantage logic, bRollInProgress order fix), and `USessionHUDComponent` (AddRollResultToChat now takes EDiceRollMode, chat message reflects mode). Added two new gotchas: bRollInProgress timing and USTRUCT UObject pointer UPROPERTY requirement.

*2026-03-20 (updated 1)* — Full camera system implemented: WASD movement along pawn forward/right vectors, middle-mouse pan (yaw + clamped pitch), scroll zoom, sprint multiplier, pan reset. Editor property validation via PostEditChangeProperty. Added IA_CameraPanReset, IA_CameraSprint. ASessionController description updated.

*2026-03-20 (updated)* — Chat input system overhauled: click-to-focus, stay-in-chat after send, click-outside/Escape to exit. Switched from hard-coded BindKey to Enhanced Input (IMC_Session, IMC_Chat, four IAs). GameplayHUDComponent now caches PlayerControllerRef and InputSubsystemRef. EnhancedInput added to Build.cs. Added Content/Input/ folder structure.

*2026-03-20* — Added roadmap items: dice physics tuning (Phase 1) and custom UI art assets (Phase 6).

*2026-04-12* — Renamed all "Gameplay"-prefixed classes to "Session": `AGameplayGameMode` → `ASessionGameMode`, `AGameplayController` → `ASessionController`, `UGameplayHUDComponent` → `USessionHUDComponent`, `AGameplayPawn` → `ASessionPawn`. All Blueprint assets reparented accordingly (GM_Session, PC_Session, P_Session, BP_SessionHUDComponent). Input folder renamed `Input/Gameplay/` → `Input/Session/`. Chat bug logged: second message can't be entered after first send.

*2026-04-12 (updated)* — Player identity / login options flow implemented. Added `UPlayerSave` (SaveLoad/) — stores permanent `FGuid PlayerID`, created on first launch in `USessionInstance::Init`. `USessionInstance` updated: loads/generates `PlayerID` on startup via `Init`. `UCampaignManagerScreen::OnCampaignSelected` builds travel URL with `?PlayerID=<guid>`. `ASessionGameMode::PostLogin` parses `PlayerID` from `OptionsString` and sets it on `ASessionPlayerState` before role checks. `USessionHUDComponent` constructor fixed: `SetIsReplicated` → `SetIsReplicatedByDefault`. Flow diagram added to `Flow Diagrams/LoginFlow.drawio`.

*2026-03-19* — Roll results display in chat complete. Dice physics improved (PhysicalMaterial, angular impulse, post-settle lock). Failsafe destroy system added with chat notification. DiceSelector count display resets correctly after rolling.

*2026-03-18* — Chat system complete (UChatBox, UChatEntry, USessionHUDComponent fully converted from Blueprint to C++, networking RPCs working).

---

Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
