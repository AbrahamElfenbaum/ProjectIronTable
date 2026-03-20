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
├── Dice/        — Dice actors and data assets
├── UI/          — Widget classes
└── Utility/     — Function libraries and general-purpose helpers
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
├── UI/
│   ├── Dice/                   — Dice widget elements (WE_DiceSelector, WE_DiceSelectorManager)
│   ├── HUD/                    — HUD widget elements (WE_ChatBox, WE_ChatEntry)
│   ├── Screens/                — Full screen widgets (W_GameplayScreen)
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
| Widget              | W_     |
| Widget Element      | WE_    |
| Enumeration         | E_     |
| Material            | M_     |
| Material Instance   | MI_    |

---

## C++ Classes

### Dice/
- **`ABaseDiceActor`** — Parent class for all dice actors. Handles mesh assignment, physics sleep detection, and roll result reporting. All individual die types inherit from this. Exposes `Mass`, `PhysicalMaterial`, `LinearDamping`, `AngularDamping`, `ImpulseRange`, `AngularImpulseRange`, and `FailSafeTime` (`float`, default 10s). On sleep, disables physics simulation on the settled mesh so it cannot be pushed again. Broadcasts `OnFailsafeDestroy` (with `EDiceType`) if a mesh hasn't settled within `FailSafeTime`. For two-mesh dice, Mesh2 receives an additional randomized offset impulse for varied separation.
- **`UDiceData`** — `UPrimaryDataAsset` subclass. Stores per-die configuration data (mesh, faces, type, etc.).

### UI/
- **`UDiceSelector`** — `UUserWidget` subclass. Requires bound widgets: `TypeText`, `CountText` (`UTextBlock`), `IncreaseButton`, `DecreaseButton` (`UButton`). Exposes `DiceClass` (`TSubclassOf<ABaseDiceActor>`), `DiceType` (`EDiceType`), and `DiceCount` (`int32`, visible/read-only). Button clicks bound in `NativeConstruct`. All logic is in C++ — the Blueprint exists only for layout and styling.
- **`UDiceSelectorManager`** — `UUserWidget` subclass. Requires bound widgets: `D4`, `D6`, `D8`, `D10`, `D12`, `D20`, `D100` (`UDiceSelector`), `RollButton` (`UButton`). Exposes `StartingLocation`, `Impulse`, `AngularImpulse` (`FVector`), `ImpulseRange`, `AngularImpulseRange` (`float`), and `TimeBeforeDestroyingDice` (`float`, default 5s) in the inspector. Selectors array is built in `NativeConstruct`. Each die spawns at `StartingLocation` with a random rotation and unit scale, then has a randomized impulse applied via `GetRandomizedVector`. Collects results via delegate, broadcasts `OnAllDiceRolled` when all dice settle, then destroys actors after the configured delay. Broadcasts `OnDiceFailsafeDestroyed` (with `EDiceType`) when a die is lost to the failsafe.
- **`UChatBox`** — `UUserWidget` subclass. Chat log display. Requires bound widgets: `ScrollBox` (`UScrollBox`), `EditableText` (`UEditableText`). Holds a `ChatEntryClass` (`TSubclassOf<UChatEntry>`) set in the editor. Handles focus, scroll, message display, and text commit input. Gets `UGameplayHUDComponent` reference via owning player controller in `NativeConstruct`.
- **`UChatEntry`** — `UUserWidget` subclass. Single chat message row. Requires bound widget: `TextBlock` (`UTextBlock`). Exposes `Message` (`FString`, expose on spawn). Sets text in `NativeConstruct`.
- **`UGameplayHUDComponent`** — `UActorComponent` subclass. Manages HUD widget lifecycle and chat networking. Creates `GameplayScreenClass` widget on `BeginPlay` (local clients only), grabs `DiceSelectorManagerRef` and `ChatBoxRef` via `GetWidgetFromName`. Binds to `OnAllDiceRolled` and `OnDiceFailsafeDestroyed` on the `DiceSelectorManager`. Has Server RPC `SendChatMessageOnServer` (broadcasts to all clients) and Client RPC `AddChatMessageOnOwningClient`. `AddRollResultToChat` formats roll results as "[Player] Rolled:\n[value] on a [type]" per die. `OnDiceFailsafeHandler` sends "[Player] lost a [type] to the void".

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
- [ ] Tune dice physics settings (mass, damping, impulse values) so rolls look and feel realistic
- [ ] Sound effects for dice rolls
- [ ] Visual effects for dice rolls
- [ ] Custom dice support (user-importable meshes and face values)
- [ ] Replace asset pack with original dice models (custom Blender meshes)

---

### Phase 2 — Core Game Setup
Establish the game framework and player interaction foundation.

- [x] Test Game Mode (`GM_Testing`)
- [x] Test Player Controller (`PC_Testing`)
- [x] Production Game Mode (`GM_Gameplay`)
- [x] Production Player Controller (`PC_Gameplay`)
- [x] HUD component (`BP_HUDComponent`) — complete, reparented to `UGameplayHUDComponent`
- [ ] Basic camera system (top-down / isometric view)
- [ ] Scene/session management (start, load, save)

---

### Phase 3 — Maps
Virtual tabletop map system.

- [ ] Map tile system
- [ ] Grid overlay (square and hex)
- [ ] Map loading and display
- [ ] User-importable map tiles and images
- [ ] Fog of war (optional / toggleable)
- [ ] Lighting and atmosphere controls

---

### Phase 4 — Miniatures
Character and creature representation on the map.

- [ ] Base miniature actor/pawn (TBD — Actor vs Pawn)
- [ ] Miniature placement and movement on grid
- [ ] Rigging system for miniature animations
- [ ] User-importable miniature meshes
- [ ] Mini labels (name, HP, status)

---

### Phase 5 — D&D 5e / 2024 Game System
First full game system implementation.

- [ ] Character sheet (stats, skills, proficiencies, HP, etc.)
- [ ] Initiative tracker / turn order system
- [ ] Spell slot tracking
- [ ] Condition tracking (Poisoned, Stunned, etc.)
- [ ] Attack rolls, saving throws, skill checks tied to character stats
- [ ] Monster/NPC stat blocks
- [ ] Basic combat flow

---

### Phase 6 — Audio & Visual Polish
Immersive presentation layer.

- [ ] Replace default Unreal UI assets with custom art (buttons, panels, screens)
- [ ] Sound effects system (dice, movement, ambience)
- [ ] Visual effects system (spells, hits, status effects)
- [ ] User-importable sounds and VFX
- [ ] Music / ambience support
- [ ] User-uploadable music (players can load and play their own audio tracks during a session)

---

### Phase 7 — Custom Content & Extensibility
Allow users to bring their own assets and game rules.

- [ ] Custom asset import pipeline (sounds, maps, minis, effects)
- [ ] Custom dice import
- [ ] Plugin/mod support (TBD)
- [ ] Support for additional TTRPG systems beyond D&D 5e
- [ ] Player-to-player asset sharing (asset owners can optionally share custom files — minis, maps, sounds — with other players in a session)

---

### Future / Backlog (Unscheduled)
Ideas to revisit later.

- Multiplayer / networking
- Campaign management (sessions, notes, loot)
- Integrated rulebook reference
- AI game master assistant
- Mobile or web companion app

---

*Last updated: 2026-03-20* — Added roadmap items: dice physics tuning (Phase 1) and custom UI art assets (Phase 6).

---

Copyright 2026 Abraham Elfenbaum. All Rights Reserved.

