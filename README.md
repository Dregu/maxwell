# MAXWELL

MAXWELL is an ingame ImGui overlay for the game ANIMAL WELL, featuring an inventory editor, warping map, cheats etc.

It has been tested on the 2024-05-28 version 1.0.0.16 on Windows 10. This is the only version officially supported by the latest autobuild, but it might work for older or newer versions too. There might also be compatibility releases for older versions available in the release section, but those probably won't have the latest features.

**Please go away if you haven't finished the game, this will spoil everything for you.**

## Disclaimer

I shouldn't have to tell you, but you should back up your save file before using a random experimental tool you found on the internet, but there you go.
This can cause irreversible damage to your save file, but probably won't if you don't do it on purpose.
If you care about your Steam achievements, you should probably use some kind of [Steam Emulator](https://mr_goldberg.gitlab.io/goldberg_emulator/) before poking around with this, as it doesn't block achievements yet.
Your antivirus will probably not like this tool because of the dll injection aspect, but you can handle that like you want to.
This is not a virus, but you probably shouldn't believe a random file on the internet telling you that either...

## Usage

Build with cmake or get the [Autobuild](https://github.com/Dregu/maxwell/releases/tag/autobuild). Put MAXWELL folder in the game folder and run from there. Check `--help` for command line switches for shortcuts.

## Features

- Spoilers
- Please go away if you haven't finished the game
- Edit most things in player inventory in real time
- Input any coordinates to warp
- Right click anywhere on screen to teleport
- Minimap
  - Right click to warp to rooms
  - Reveal whole minimap ingame
  - Warp to other dimensions
  - Show other dimension outlines
  - Track wheel position
- Godmode
- Noclip without cring
- Save anywhere
- Adjust window scaling
- Disable fog of war / darkness
- Play in b&w gameboy mode
- Hide the game hud
- Take unobstructed screenshots for mapping
- Pause and frame advance
  - Ctrl+Tab to toggle
  - Tab to skip one frame
- Rudimentary runtime level editor
  - Change water level
  - Middle click to pick tile from room
  - Left click to place tile
  - Static tiles will update instantly, dynamic tiles need a room reload
  - Very WIP
- Bunny sequencer
  - Plays the flute automatically based on pixels on the mural
  - Blues and reds are half notes, beiges full notes
  - Take out the flute to play your song while in the room
