# SwanGBA V0.6.5

<img align="right" width="220" src="./logo.png" />

This is a Bandai WonderSwan (Color/Crystal) & Benesse PocketChallenge V2 emulator for the Nintendo GBA.

## How to use

When the emulator starts, you press L+R to open up the menu.
Now you can use the cross to navigate the menus, A to select an option,
B to go back a step.

## Menu

### File

* Load Game: Select a game to load.
* Load State: Load a previously saved state of the currently running game.
* Save State: Save a state of the currently running game.
* Load NVRAM: Load non volatile ram (EEPROM/SRAM) for the currently running game.
* Save NVRAM: Save non volatile ram (EEPROM/SRAM) for the currently running game.
* Save Settings: Save the current settings (and internal EEPROM).
* Reset Game: Reset the currently running game.

### Controller

* Autofire: Select if you want autofire.
* Swap A-B: Swap which GBA button is mapped to which WS button.

### Display

* Gamma: Lets you change the gamma ("brightness").
* Contrast: Lets you change the contrast.
* B&W Palette: Here you can select the palette for B & W games.

### Settings

* Speed: Switch between speed modes.
  * Normal: Game runs at it's normal speed.
  * 200%: Game runs at double speed.
  * Max: Games can run up to 4 times normal speed (might change).
  * 50%: Game runs at half speed.
* Autoload State: Toggle Savestate autoloading. Automagically load the savestate associated with the selected game.
* Autoload NVRAM: Toggle EEPROM/SRAM autoloading. Automagically load the EEPROM/SRAM associated with the selected game.
* Autosave Settings: This will save settings when leaving menu if any changes are made.
* Autopause Game: Toggle if the game should pause when opening the menu.
* Overclock EWRAM: Changes the waitstates on EWRAM between 2 and 1, might damage your GBA and uses more power, around 10% speedgain. Use at your own risk!
* Autosleep: Change the autosleep time, also see Sleep.

### Machine

* Machine: Select the emulated machine.
* Change Battery: Change to a new main battery (AA/LR6).
* Clear internal EEPROM: Reset internal EEPROM.
* Headphones: Select whether heaphones are connected or not.
* Cpu speed hacks: Allow speed hacks.
* Half Cpu Speed: Just half the emulated cpu speed.

### Debug

* Debug Output: Show FPS and logged text.
* Disable Foreground: Turn on/off foreground rendering.
* Disable Background: Turn on/off background rendering.
* Disable Sprites: Turn on/off sprite rendering.
* Step Frame: Emulate one frame.

### About

Some info about the emulator and game...

### Sleep

Put the GBA into sleepmode. START+SELECT wakes up from sleep mode (activated
 from this menu or from	5/10/30	minutes of inactivity).

## Controls

In horizontal games:
* D-pad is mapped to WS X1-X4.
* A & B buttons are mapped to WS A & B.
* Holding L maps the D-pad to WS Y1-Y4.

In vertical games:
* D-pad is mapped to WS Y1-Y4.
* A, B are mapped to WS X3,X4.

## Games

There are 2 games that I know of that has serious problems.

* Chou Denki Card Game: You need to initialize NVRAM, this is the last item on the first page (初期化).
* Mahjong Touryuumon, speed too fast.

There are a couple of games that have visual glitches.

* Digimon - Anode Tamer & Cathode Tamer, missing background gradient in battles.
* Final Fantasy, Sprites show in dialog windows.
* Final Lap 2000, incorrect road colors.
* Final Lap Special - GT & Formula Machine, incorrect road colors.
* From TV Animation One Piece - Grand Battle Swan Colosseum, incorrect sky color.
* Makaimura, first boss sprites are glitchy, gargoyles in intro should not show up on the right.
* Romancing Sa-Ga, sprites overlap text boxes.
* Sorobang, needs all 1024 tiles in 4color mode.
* WonderSwanColor BIOS, needs all 1024 tiles in 4color mode.

## Credits

```text
Huge thanks to Loopy for the incredible PocketNES, without it this emu would probably never have been made.
Thanks to:
Ed Mandy (Flavor) for WonderSwan info & flashcart. https://www.flashmasta.com
Koyote for WonderSwan info.
Alex Marshall (trap15) for WonderSwan info. http://daifukkat.su/docs/wsman/
Guy Perfect for WonderSwan info http://perfectkiosk.net/stsws.html
asie for info and inspiration.
Godzil for the boot rom stubs. https://github.com/Godzil/NewOswan
lidnariq for RTC info.
plasturion for some BnW palettes.
Dwedit for help and inspiration with a lot of things.
```

Fredrik Ahlström

Twitter @TheRealFluBBa

http://www.github.com/FluBBaOfWard
