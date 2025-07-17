# MAXWELL

MAXWELL is an ingame ImGui overlay for the game ANIMAL WELL, featuring an inventory editor, warping map, cheats etc.

It has been tested on the 2024-10-31 version 1.0.0.20 on Windows 10. This is the only version officially supported by the latest autobuild, but it might work for older or newer versions too. There might also be compatibility releases for older versions available in the release section, but those probably won't have the latest features.

**Please go away if you haven't finished the game, this will spoil everything for you.**

## Disclaimer

I shouldn't have to tell you, but you should back up your save file before using a random experimental tool you found on the internet, but there you go.
This can cause irreversible damage to your save file, but probably won't if you don't do it on purpose.
If you care about your Steam achievements, you should probably use some kind of [Steam Emulator](https://mr_goldberg.gitlab.io/goldberg_emulator/) before poking around with this, as it doesn't block achievements yet.
Your antivirus will probably not like this tool because of the dll injection aspect, but you can handle that like you want to.
This is not a virus, but you probably shouldn't believe a random file on the internet telling you that either...

## Usage

- Build with cmake or get the [Autobuild](https://github.com/Dregu/maxwell/releases/tag/autobuild)
- Just run `MAXWELL.exe`
  - If MAXWELL can't find the game automatically you can put `MAXWELL.exe` in a `MAXWELL` folder inside the game folder and run it from there
  - You can also run or create a shortcut using `MAXWELL.exe --launch_game [path]` to launch the game
  - Check `--help` for other command line switches
- `MAXWELL.dll` also works with other generic DLL injectors
- Works with Wine 9 / Proton 9
- You can rename `MAXWELL.dll` to `xinput9_1_0.dll` and place it next to `Animal Well.exe` to load it without the injector
  - Use the Steam launch options `PROTON_USE_WINED3D=1 WINEDLLOVERRIDES="xinput9_1_0=n,b" %command%` to load this on Proton
- Doesn't seem to work with some builds of Goldberg's emulator if it has overlay enabled
  - To fix this, create a `Animal Well/steam_settings/disable_overlay.txt` file

## Features

- Spoilers
- Please go away if you haven't finished the game
- Edit most things in player inventory in real time
- Input any coordinates to warp
- Right click anywhere on screen to teleport
- Godmode
- Noclip without cring
- Save anywhere
- Adjust window scaling
- Disable fog of war / darkness
- Disable spot lights
- Force bright or funny room palette
- Hide the game hud
- Hide the player character
- Play in b&w gameboy mode
- Take unobstructed screenshots for mapping
- Always Groundhog Day mode
- Minimap
  - Right click to warp to rooms
  - Reveal whole minimap ingame
  - Warp to other dimensions
  - Show other dimension outlines
  - Track wheel position
  - Show destroyed tiles
- Pause and frame advance
  - PgUp to toggle
  - PgDn to skip one frame
- Rudimentary runtime level editing
  - WIP
  - Change water level
  - Middle click to pick tile from room
  - Left click to place tile
  - Mouse4 to mark tiles destroyed
  - Hold shift for background layer or fixing destroyed tiles
  - Search for tiles by ID in the current map
  - Highlight and mass replace found tiles
  - Import map files exported from the [level editor](https://github.com/Redcrafter/Animal-Well-editor) at runtime
    - Static tiles will update instantly, dynamic tiles need a room reload
- Bunny sequencer
  - WIP
  - Plays the flute automatically based on pixels on the mural
  - Blues and reds are half notes, beiges full notes
  - Take out the flute to play your song while in the room
- Keyboard remapping
  - Disable all undocumented game keys and remap your own
  - All hud elements should also display correctly for most keys (all alphanumeric + space)
  - Numpad is left untouched at the moment and works normally

### Mods

  - WIP, untested, these things might still change on a whim
  - Loads correctly named and formatted files from `MAXWELL/Mods`
  - No error checking
  - All original maps, assets and textures can be dumped with MAXWELL or the [level editor](https://github.com/Redcrafter/Animal-Well-editor)
    - Edited current state of maps can also be dumped with MAXWELL
    - Edited light params (asset 179) can also be dumped with MAXWELL
  - Supports 2 types of modded files
    - Full asset mods in `MAXWELL/Mods/[Mod Name]/Assets/Y.whatever` where `Y` is an asset number as dumped by the editor
    - Texture atlas mods in `MAXWELL/Mods/[Mod Name]/Tiles/Z.png` where `Z` is a tile number. The texture will overwrite the atlas at the position given by `254.tiles`. Tile mods can apply on top of `255.png` loaded by other mods.
  - Conflicting mods will be highlighted in the mods menu, the first encountered assets will be loaded
  - Behavior with unexpected file formats is almost definitely crashing
  - Examples:
    - `MAXWELL/Mods/Epic mod/Assets/101.ogg` replaces dog barks with farts
    - `MAXWELL/Mods/Epic mod/Assets/179.ambient` replaces the light params
    - `MAXWELL/Mods/Epic mod/Assets/300.map` replaces well map
    - `MAXWELL/Mods/Epic mod/Tiles/416.png` replaces only the beanie texture in main texture atlas
    - `MAXWELL/Mods/Epic mod/Tiles/365.png` replaces another thing in main texture atlas
  - These are the only asset file types currently tested, but others might also work
  - **MAXWELL.exe has to be the one launching the game to hook the asset loading functions early**
    - Mods won't load correctly if you just launch the game from Steam and have MAXWELL searching for processes
