# SwanGBA V0.3.8

<img align="right" width="220" src="./logo.png" />

This is a Bandai WonderSwan (Color/Crystal) & PocketChallenge V2 emulator for the Nintendo GBA.

## How to use:

When the emulator starts, you press L+R to open up the menu.
Now you can use the cross to navigate the menus, A to select an option,
B to go back a step.

## Menu:

### File:
	Load Game: Select a game to load.
	Load State: Load a previously saved state of the currently running game.
	Save State: Save a state of the currently running game.
	Load NVRAM: Load non volatile ram (EEPROM/SRAM) for the currently running game.
	Save NVRAM: Save non volatile ram (EEPROM/SRAM) for the currently running game.
	Save Settings: Save the current settings (and internal EEPROM).
	Reset Game: Reset the currently running game.

### Options:
	Controller:
		Autofire: Select if you want autofire.
		Controller: 2P start a 2 player game.
		Swap A/B: Swap which GBA button is mapped to which WS button.
	Display:
		Mono Palette: Here you can select the palette for B & W games.
		Gamma: Lets you change the gamma ("brightness").
		Disable Foreground: Turn on/off foreground rendering.
		Disable Background: Turn on/off background rendering.
		Disable Sprites: Turn on/off sprite rendering.
	Machine Settings:
		Machine: Select the emulated machine.
		Select WS Bios: Load a real WS Bios.
		Select WS Color Bios: Load a real WS Color Bios.
		Select WS Crystal Bios: Load a real WS Crystal Bios.
		Import internal EEPROM: Load a special internal EEPROM.
		Clear internal EEPROM: Reset internal EEPROM.
		Cpu speed hacks: Allow speed hacks.
		Change Battery: Change to a new main battery (AA/LR6).
		Language: Select between Japanese and English.
	Settings:
		Speed: Switch between speed modes.
			Normal: Game runs at it's normal speed.
			200%: Game runs at double speed.
			Max: Games can run up to 4 times normal speed (might change).
			50%: Game runs at half speed.
		Autoload State: Toggle Savestate autoloading.
			Automagically load the savestate associated with the selected game.
		Autoload NVRAM: Toggle EEPROM/SRAM autoloading.
			Automagically load the EEPROM/SRAM associated with the selected game.
		Autosave Settings: This will save settings when
			leaving menu if any changes are made.
		Autopause Game: Toggle if the game should pause when opening the menu.
		Debug Output: Show an FPS meter for now.
		Autosleep: Change the autosleep time, also see Sleep.

### About:
	Some info about the emulator and game...

### Sleep:
	Put the GBA into sleepmode.
	START+SELECT wakes up from sleep mode (activated from this menu or from	5/10/30	minutes of inactivity).

## Controls:
In horizontal games the d-pad is mapped to WS X1-X4. A & B buttons are mapped to WS A & B.
Holding L maps the dpad to WS Y1-Y4.

In vertical games the d-pad is mapped to WS Y1-Y4. A, B, X & Y are mapped to WS X1-X4.

## Credits:

Huge thanks to Loopy for the incredible PocketNES, without it this emu would
probably never have been made.
Thanks to:
Ed Mandy (Flavor) for WonderSwan info & flashcart. https://www.flashmasta.com
Koyote for WonderSwan info.
Alex Marshall (trap15) for WonderSwan info. http://daifukkat.su/docs/wsman/
Godzil for the boot rom stubs. https://github.com/Godzil/NewOswan
lidnariq for RTC info.
Dwedit for help and inspiration with a lot of things.


Fredrik Ahlström

Twitter @TheRealFluBBa

http://www.github.com/FluBBaOfWard
