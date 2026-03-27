# ProjectIronTable

A virtual tabletop (VTT) simulator built in Unreal Engine 5.7. ProjectIronTable lets a group of players run a tabletop RPG session online — rolling physics-based dice, moving miniatures on a shared map, tracking initiative and character stats, and communicating through an in-session chat. Designed to feel like a real tabletop, not a video game.

**Primary game system:** D&D 5e / 2024 (expandable to other TTRPGs)
**Platform:** PC (Windows)
**Status:** Active development

### Features (planned)
- Physics-based dice rolling with full result history
- Private and public chat with dice result integration
- Collaborative shared notes
- Grid-based maps (flat image and tile-based formats)
- 3D miniatures with fog of war and vision effects
- Initiative tracker with GM staging list
- D&D 5e character sheets, spell management, and condition tracking
- Per-player customizable UI with draggable, resizable panels
- Custom user-imported content (maps, minis, dice, sounds)
- Multiplayer with separate GM and host roles

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
- `Chat/` — Chat widget classes (`UChatBox`, `UChatEntry`, `UChatChannel`, `UChatTab`)
- `Dice/` — Dice actors (`ABaseDiceActor`, `ADiceSpawnVolume`) and data assets (`UDiceData`)
- `Pawns/` — Camera pawn (`AGameplayPawn`)
- `PlayerControllers/` — Player controller (`AGameplayController`)
- `PlayerList/` — Player list widget classes (`UPlayerList`, `UPlayerRow`)
- `UI/` — Non-chat widget classes (`UDiceSelector`, `UDiceSelectorManager`, `UGameplayHUDComponent`)
- `Physics/Materials/` — Physical material assets (friction, restitution for dice)
- `Utility/` — Blueprint function libraries (`UFunctionLibrary`)

### Content (`Content/`)
- `Blueprints/Core/GameModes/` — Game mode blueprints (`GM_Testing`, `GM_Gameplay`)
- `Blueprints/Core/PlayerControllers/` — Player controller blueprints (`PC_Testing`, `PC_Gameplay`)
- `Blueprints/Core/Components/` — Actor components (`BP_HUDComponent`)
- `Blueprints/Pawns/` — Camera pawn Blueprint (`P_GameplayPawn`)
- `Blueprints/Dice/` — Base dice actor Blueprint (`A_BaseDiceActor`)
- `Blueprints/Dice/DiceActors/` — Individual die blueprints
- `Blueprints/Utility/` — Utility blueprints
- `Data/DataAssets/Dice/` — Dice data assets
- `Levels/Dev/` — Development/test levels
- `Materials/Dice/` — Dice materials
- `Meshes/Dice/` — Dice meshes (*Dungeons of Dice* by NNJohn)
- `Textures/` — Texture assets
- `Input/Gameplay/` — `IMC_Gameplay`, `IA_CameraMove`, `IA_CameraPan`, `IA_CameraPanReset`, `IA_CameraSprint`, `IA_CameraZoom`, `IA_FocusChat`
- `Input/Chat/` — `IMC_Chat`, `IA_ExitChat`, `IA_ScrollChat`
- `UI/Dice/` — Dice widget elements (`WE_DiceSelector`, `WE_DiceSelectorManager`)
- `UI/Chat/` — Chat widgets (`W_ChatBox`, `WE_ChatChannel`, `WE_ChatTab`, `WE_ChatEntry`)
- `UI/PlayerList/` — Player list widgets (`W_PlayerList`, `WE_PlayerRow`)
- `UI/Screens/` — Full screen widgets (`S_GameplayScreen`)
- `UI/Testing/` — Debug/test widgets (`WE_DebugDisplay`)
