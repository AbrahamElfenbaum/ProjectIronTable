# ProjectIronTable

Unreal Engine 5.7 — TTRPG simulator (maps, minis, dice rolling).

## Requirements
- Unreal Engine 5.7
- Git LFS (run `git lfs install` after cloning)

## Structure

### Source (`Source/ProjectIronTable/`)
- `Dice/` — Dice actors (`ABaseDiceActor`) and data assets (`UDiceData`)
- `UI/` — Widget classes (`UDiceSelector`, `UDiceSelectorManager`)
- `Utility/` — Blueprint function libraries (`UFunctionLibrary`)

### Content (`Content/`)
- `Blueprints/Core/GameModes/` — Game mode blueprints (`GM_Testing`)
- `Blueprints/Core/PlayerControllers/` — Player controller blueprints (`PLC_Testing`)
- `Blueprints/Core/Components/` — Actor components (`BP_HUDComponent`)
- `Blueprints/Dice/DiceActors/` — Dice actor blueprints
- `Blueprints/Utility/` — Utility blueprints
- `Data/DataAssets/Dice/` — Dice data assets
- `Levels/Dev/` — Development/test levels
- `Materials/Dice/` — Dice materials
- `Meshes/Dice/` — Dice meshes (*Dungeons of Dice* by NNJohn)
- `Textures/` — Texture assets
- `UI/Dice/` — Dice UI widgets
