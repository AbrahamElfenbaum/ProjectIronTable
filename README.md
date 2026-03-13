# ProjectIronTable
   2
   3 Unreal Engine 5.7 — TTRPG simulator (maps, minis, dice rolling).
   4
   5 ## Requirements
   6 - Unreal Engine 5.7
   7 - Git LFS (run `git lfs install` after cloning)
   8
   9 ## Structure
  10
  11 ### Source (`Source/ProjectIronTable/`)
  12 - `Dice/` — Dice actors (`ABaseDiceActor`) and data assets (`UDiceData`)
  13 - `UI/` — Widget classes (`UDiceSelector`, `UDiceSelectorManager`)
  14 - `Utility/` — Blueprint function libraries (`UFunctionLibrary`)
  15
  16 ### Content (`Content/`)
  17 - `Blueprints/Core/GameModes/` — Game mode blueprints
  18 - `Blueprints/Core/PlayerControllers/` — Player controller blueprints
  19 - `Blueprints/Dice/DiceActors/` — Dice actor blueprints
  20 - `Blueprints/Utility/` — Utility blueprints
  21 - `Data/DataAssets/Dice/` — Dice data assets
  22 - `Levels/Dev/` — Development/test levels
  23 - `Materials/Dice/` — Dice materials
  24 - `Meshes/Dice/` — Dice meshes
  25 - `Textures/` — Texture assets
  26 - `UI/Dice/` — Dice UI widgets
