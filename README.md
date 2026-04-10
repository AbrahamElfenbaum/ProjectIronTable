# ProjectIronTable

A virtual tabletop (VTT) simulator built in Unreal Engine 5.7. ProjectIronTable lets a group of players run a tabletop RPG session online — rolling physics-based dice, moving miniatures on a shared map, tracking initiative and character stats, and communicating through an in-session chat. Designed to feel like a real tabletop, not a video game.

**Primary game system:** D&D 5e / 2024 (expandable to other TTRPGs)
**Platform:** PC (Windows)
**Status:** Active development

### Features (planned)
- Physics-based dice rolling with full result history
- Private and public chat with dice result integration
- Collaborative shared notes with rich-text formatting, accessible between sessions
- **3D map builder** — place tiles and props to build environments in real time; supports elevation, multi-floor layouts, and variable tile sizes; Combat Map and World/Region Map scale modes; world maps link to combat maps via location pins
- Fog of war with GM-controlled reveal, camera boundary enforcement, and optional auto-reveal by token line of sight
- 3D miniatures with movement range enforcement, difficult terrain, and diagonal ruling options
- Initiative tracker with round tracking, staging list, skip/delay, and manual turn advancement
- Combat flow managed by GM: start, end, and entity management panel for NPCs, summoned creatures, and companions
- D&D 5e character sheets, spell management, and condition tracking; game system plugin architecture supports additional TTRPGs
- Character creation with root layer (universal) and game system layer (ruleset-specific)
- Inventory and loot system with GM item list, send-to-player flow, and custom item creation
- Campaign Manager — browse, create, and manage campaigns; public browser with filters; private/public campaigns; invite codes; pre-session lobby
- Player profiles with username, bio, and games played (app history + self-reported)
- Scheduling system for session frequency, meeting days, and session length
- Session save/load with autosave, configurable intervals, and on-close save
- Measurement tools (ruler, AoE templates) universal across game systems
- Sound and music — ambient, music, and proximity-based SFX; map-baked audio; built-in asset library
- Per-player customizable UI with draggable, resizable panels
- Custom user-imported 3D assets (tiles, props, minis, dice, sounds) via glTF; auto-distributed to all session clients on map load; built-in library included
- Multiplayer with separate Server Owner and Host roles; listen server model

## Credits
- **Kenney Impact Sounds** — [kenney.nl](https://www.kenney.nl) (CC0)
- **Dungeons of Dice** mesh pack — NNJohn

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
- `AssetLibrary/` — Asset library screen (`UAssetLibraryScreen`, stub)
- `CampaignBrowser/` — Campaign browser screen (`UCampaignBrowserScreen`, stub)
- `CampaignManager/` — Campaign manager widget classes (`UGameTypeButton`, `UCampaignCard`, `UCampaignManagerScreen`)
- `Chat/` — Chat widget classes (`UChatBox`, `UChatEntry`, `UChatChannel`, `UChatTab`, `UChatChannelListEntry`)
- `Components/` — Actor component classes (`UGameplayHUDComponent`, `UMainScreenHUDComponent`)
- `Dice/` — Dice actors (`ABaseDiceActor`, `ADiceSpawnVolume`) and data assets (`UDiceData`)
- `GameModes/` — Game mode classes (`AGameplayGameMode`)
- `GameStates/` — Game state classes (`ASessionGameState`)
- `Pawns/` — Camera pawn (`AGameplayPawn`)
- `PlayerControllers/` — Player controllers (`AGameplayController`, `AMainScreenController`)
- `PlayerList/` — Player list widget classes (`UPlayerList`, `UPlayerRow`)
- `PlayerStates/` — Player state classes (`ASessionPlayerState`)
- `SaveLoad/` — Save game classes (`UPanelLayoutSave`, `UCameraSettingsSave`, `UCampaignManagerSave`, `USessionSave`)
- `Settings/` — Settings widget classes (`UCameraSettingsPanel`, `USettingsScreen`)
- `UI/` — Non-chat widget classes (`UBaseScreen`, `UDiceSelector`, `UDiceSelectorManager`, `UTaskbar`, `UTaskbarButton`, `UDraggablePanel`, `UDragHandle`, `UResizeHandle`, `USettingsSlider`, `UHomeScreen`)
- `Physics/Materials/` — Physical material assets (friction, restitution for dice)
- `Utility/` — Blueprint function libraries (`UFunctionLibrary`, `UDelegateLibrary`)

### Content (`Content/`)
- `Blueprints/Core/GameModes/` — Game mode blueprints (`GM_Testing`, `GM_Gameplay`, `GM_MainScreen`)
- `Blueprints/Core/PlayerControllers/` — Player controller blueprints (`PC_Testing`, `PC_Gameplay`, `PC_MainScreen`)
- `Blueprints/Core/Components/` — Actor components (`BP_HUDComponent`, `BP_HomeScreenHUDComponent`)
- `Blueprints/Pawns/` — Camera pawn Blueprint (`P_GameplayPawn`)
- `Blueprints/Dice/` — Base dice actor Blueprint (`A_BaseDiceActor`)
- `Blueprints/Dice/DiceActors/` — Individual die blueprints
- `Blueprints/Utility/` — Utility blueprints
- `Data/DataAssets/Dice/` — Dice data assets
- `Levels/Dev/` — Development/test levels (`Dev_` prefix)
- `Levels/HomeScreen/` — Home screen level (`L_HomeScreen`)
- `Levels/Gameplay/` — Gameplay levels (`L_Gameplay`)
- `Materials/Dice/` — Dice materials
- `Audio/Dice/` — Dice collision sound assets (Kenney Impact Sounds, CC0)
- `Meshes/Dice/` — Dice meshes (*Dungeons of Dice* by NNJohn)
- `Textures/` — Texture assets
- `Input/Gameplay/` — `IMC_Gameplay`, `IA_CameraMove`, `IA_CameraPan`, `IA_CameraPanReset`, `IA_CameraSprint`, `IA_CameraZoom`, `IA_FocusChat`
- `Input/Chat/` — `IMC_Chat`, `IA_ExitChat`, `IA_ScrollChat`
- `UI/Dice/` — Dice widget elements (`WE_DiceSelector`, `WE_DiceSelectorManager`)
- `UI/Chat/` — Chat widgets (`W_ChatBox`, `WE_ChatChannel`, `WE_ChatTab`, `WE_ChatEntry`)
- `UI/PlayerList/` — Player list widgets (`W_PlayerList`, `WE_PlayerRow`)
- `UI/Taskbar/` — Taskbar widgets (`W_Taskbar`, `WE_TaskbarButton`)
- `UI/Utility/` — Reusable utility widgets (`W_DraggablePanel`, `WBP_DragHandle`, `WBP_ResizeHandle`)
- `UI/CampaignManager/` — Campaign manager widget elements (`WE_GameTypeButton`, `WE_CampaignCard`)
- `UI/Screens/` — Full screen widgets (`S_GameplayScreen`, `S_HomeScreen`, `S_MainScreen`, `S_SettingsScreen`, `S_CampaignManagerScreen`, `S_CampaignBrowserScreen`, `S_AssetLibraryScreen`)
- `UI/Settings/` — Settings widget elements (`WE_SettingsSlider`)
- `UI/Settings/Panels/` — Settings panel widgets (`WE_CameraSettingsPanel`)
- `UI/Testing/` — Debug/test widgets (`WE_DebugDisplay`)
