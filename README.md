# ProjectIronTable

Unreal Engine 5.7 — TTRPG simulator (maps, minis, dice rolling).

## Requirements
- Unreal Engine 5.7
- Git LFS (run `git lfs install` after cloning)

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

> Never let Unreal compile C++ on its own — Blueprint widgets inheriting from C++ classes will show "invalid parent class" warnings if the editor loads before C++ classes are registered.

## Structure

### Source (`Source/ProjectIronTable/`)
- `Dice/` — Dice actors (`ABaseDiceActor`) and data assets (`UDiceData`)
- `UI/` — Widget classes (`UDiceSelector`, `UDiceSelectorManager`, `UChatBox`, `UChatEntry`, `UGameplayHUDComponent`)
- `Physics/Materials/` — Physical material assets (friction, restitution for dice)
- `Utility/` — Blueprint function libraries (`UFunctionLibrary`)

### Content (`Content/`)
- `Blueprints/Core/GameModes/` — Game mode blueprints (`GM_Testing`, `GM_Gameplay`)
- `Blueprints/Core/PlayerControllers/` — Player controller blueprints (`PC_Testing`, `PC_Gameplay`)
- `Blueprints/Core/Components/` — Actor components (`BP_HUDComponent`)
- `Blueprints/Dice/` — Base dice actor Blueprint (`A_BaseDiceActor`)
- `Blueprints/Dice/DiceActors/` — Individual die blueprints
- `Blueprints/Utility/` — Utility blueprints
- `Data/DataAssets/Dice/` — Dice data assets
- `Levels/Dev/` — Development/test levels
- `Materials/Dice/` — Dice materials
- `Meshes/Dice/` — Dice meshes (*Dungeons of Dice* by NNJohn)
- `Textures/` — Texture assets
- `UI/Dice/` — Dice widget elements (`WE_DiceSelector`, `WE_DiceSelectorManager`)
- `UI/HUD/` — HUD widget elements (`WE_ChatBox`, `WE_ChatEntry`)
- `UI/Screens/` — Full screen widgets (`W_GameplayScreen`)
- `UI/Testing/` — Debug/test widgets (`WE_DebugDisplay`)
