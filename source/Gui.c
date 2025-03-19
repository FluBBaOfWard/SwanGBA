#include <gba.h>
#include <string.h>

#include "Gui.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Main.h"
#include "FileHandling.h"
#include "WonderSwan.h"
#include "Cart.h"
#include "Gfx.h"
#include "Sound.h"
#include "io.h"
#include "cpu.h"
#include "WSCart/WSCart.h"
#include "ARMV30MZ/Version.h"
#include "Sphinx/Version.h"

#define EMUVERSION "V0.7.2 2025-03-19"

void hacksInit(void);

static void gammaChange(void);
static void paletteChange(void);
static const char *getPaletteText(void);
static void machineSet(void);
static const char *getMachineText(void);
static void headphonesSet(void);
static const char *getHeadphonesText(void);
static void speedHackSet(void);
static const char *getSpeedHackText(void);
static void cpuHalfSet(void);
static const char *getCPUHalfText(void);
static void borderSet(void);
static const char *getBorderText(void);
static void soundSet(void);
static void languageSet(void);
static void batteryChange(void);
static void stepFrameUI(void);
//static const char *getControllerText(void);
//static const char *getJoyMappingText(void);
static void swapABSet(void);
static const char *getSwapABText(void);
static void contrastSet(void);
static const char *getContrastText(void);
static void fgrLayerSet(void);
static const char *getFgrLayerText(void);
static void bgrLayerSet(void);
static const char *getBgrLayerText(void);
static void sprLayerSet(void);
static const char *getSprLayerText(void);
static void winLayerSet(void);
static const char *getWinLayerText(void);

static void updateGameId(char *buffer);
static void updateCartInfo(char *buffer);
static void updateMapperInfo(char *buffer);

const MItem dummyItems[] = {
	{"", uiDummy},
};
const MItem mainItems[] = {
	{"File->", ui2},
	{"Controller->", ui3},
	{"Display->", ui4},
	{"Settings->", ui5},
	{"Machine->", ui6},
	{"Debug->", ui7},
	{"About->", ui8},
	{"Sleep", gbaSleep},
	{"Reset Console", resetGame},
	{"Quit Emulator", ui10},
};
const MItem fileItems[] = {
	{"Load Game->", selectGame},
	{"Load NVRAM", loadNVRAM},
	{"Save NVRAM", saveNVRAM},
	{"Save Settings", saveSettings},
	{"Reset Game", resetGame},
};
const MItem ctrlItems[] = {
	{"B Autofire: ", autoBSet, getAutoBText},
	{"A Autofire: ", autoASet, getAutoAText},
	{"Swap A-B:   ", swapABSet, getSwapABText},
};
const MItem displayItems[] = {
	{"Gamma: ", gammaChange, getGammaText},
	{"Contrast: ", contrastSet, getContrastText},
	{"B&W Palette: ", paletteChange, getPaletteText},
	{"Border: ", borderSet, getBorderText},
};
const MItem setItems[] = {
	{"Speed: ", speedSet, getSpeedText},
	{"Autoload State: ", autoStateSet, getAutoStateText},
	{"Autosave Settings: ", autoSettingsSet, getAutoSettingsText},
	{"Autopause Game: ", autoPauseGameSet, getAutoPauseGameText},
	{"EWRAM Overclock: ", ewramSet, getEWRAMText},
	{"Autosleep: ", sleepSet, getSleepText},
};
const MItem machineItems[] = {
	{"Machine: ", machineSet, getMachineText},
	{"Change Batteries", batteryChange},
	{"Clear Internal EEPROM", clearIntEeproms},
	{"Headphones: ", headphonesSet, getHeadphonesText},
	{"Cpu Speed Hacks: ", speedHackSet, getSpeedHackText},
	{"Half Cpu Speed: ", cpuHalfSet, getCPUHalfText},
	{"Sound: ", soundSet, getSoundEnableText},
	//{"Language", languageSet},
};
const MItem debugItems[] = {
	{"Debug Output:", debugTextSet, getDebugText},
	{"Disable Foreground:", fgrLayerSet, getFgrLayerText},
	{"Disable Background:", bgrLayerSet, getBgrLayerText},
	{"Disable Sprites:", sprLayerSet, getSprLayerText},
	{"Disable Windows:", winLayerSet, getWinLayerText},
	{"Step Frame", stepFrameUI},
};
const MItem fnList9[] = {
	{"", quickSelectGame},
};
const MItem quitItems[] = {
	{"Yes", exitEmulator},
	{"No", backOutOfMenu},
};

const Menu menu0 = MENU_M("", uiNullNormal, dummyItems);
Menu menu1 = MENU_M("Main Menu", uiAuto, mainItems);
const Menu menu2 = MENU_M("File Handling", uiAuto, fileItems);
const Menu menu3 = MENU_M("Controller Settings", uiAuto, ctrlItems);
const Menu menu4 = MENU_M("Display Settings", uiAuto, displayItems);
const Menu menu5 = MENU_M("Other Settings", uiAuto, setItems);
const Menu menu6 = MENU_M("Machine Settings", uiAuto, machineItems);
const Menu menu7 = MENU_M("Debug", uiAuto, debugItems);
const Menu menu8 = MENU_M("About", uiAbout, dummyItems);
const Menu menu9 = MENU_M("Load game", uiLoadGame, fnList9);
const Menu menu10 = MENU_M("Quit Emulator?", uiAuto, quitItems);

const Menu *const menus[] = {&menu0, &menu1, &menu2, &menu3, &menu4, &menu5, &menu6, &menu7, &menu8, &menu9, &menu10 };

u8 gGammaValue = 0;
u8 gContrastValue = 3;
u8 gBorderEnable = 1;
u8 serialPos = 0;
EWRAM_BSS char serialOut[32];

const char *const machTxt[]  = {"Auto", "WonderSwan", "WonderSwan Color", "SwanCrystal", "Pocket Challenge V2"};
const char *const bordTxt[]  = {"Black", "Frame", "BG Color", "None"};
const char *const palTxt[]   = {"Classic", "Black & White", "Red", "Green", "Blue", "Green-Blue", "Blue-Green", "Puyo Puyo Tsu"};
const char *const langTxt[]  = {"Japanese", "English"};

/// This is called at the start of the emulator
void setupGUI() {
//	keysSetRepeat(25, 4);	// Delay, repeat.
	menu1.itemCount = ARRSIZE(mainItems) - (enableExit?0:1);
	closeMenu();
}

/// This is called when going from emu to ui.
void enterGUI() {
}

/// This is called going from ui to emu.
void exitGUI() {
	setupEmuBorderPalette();
}

void quickSelectGame() {
	openMenu();
	selectGame();
	closeMenu();
}

void uiNullNormal() {
	uiNullDefault();
}

void uiAbout() {
	char gameInfoString[32];
	setupSubMenuText();
	drawText("B:        WS B Button", 3);
	drawText("A:        WS A Button", 4);
	drawText("Start:    WS Start Button", 5);
	drawText("Select:   WS Sound Button", 6);
	drawText("DPad:     WS X1-X4", 7);

	updateGameId(gameInfoString);
	drawText(gameInfoString, 9);

	updateCartInfo(gameInfoString);
	drawText(gameInfoString, 10);

	updateMapperInfo(gameInfoString);
	drawText(gameInfoString, 11);

	drawText("SwanGBA    " EMUVERSION, 17);
	drawText("Sphinx     " SPHINXVERSION, 18);
	drawText("ARMV30MZ   " ARMV30MZVERSION, 19);
}

void uiLoadGame() {
	setupSubMenuText();
}

void nullUINormal(int key) {
}

void nullUIDebug(int key) {
}

void resetGame() {
	loadCart();
	setupEmuBackground();
	setupMenuPalette();
	powerIsOn = true;
}

void updateGameId(char *buffer) {
	char catalog[8];
	char2HexStr(catalog, gGameHeader->gameId);
	strlMerge(buffer, "Game Id, Rev    #: 0x", catalog, 32);
	strlMerge(buffer, buffer, " 0x", 32);
	char2HexStr(catalog, gGameHeader->gameRev);
	strlMerge(buffer, buffer, catalog, 32);
}

void updateCartInfo(char *buffer) {
	char catalog[8];
	char2HexStr(catalog, gGameHeader->romSize);
	strlMerge(buffer, "ROM Size, Save  #: 0x", catalog, 32);
	strlMerge(buffer, buffer, " 0x", 32);
	char2HexStr(catalog, gGameHeader->nvramSize);
	strlMerge(buffer, buffer, catalog, 32);
}

void updateMapperInfo(char *buffer) {
	char catalog[8];
	char2HexStr(catalog, gGameHeader->flags);
	strlMerge(buffer, "Flags, Mapper   #: 0x", catalog, 32);
	strlMerge(buffer, buffer, " 0x", 32);
	char2HexStr(catalog, gGameHeader->mapper);
	strlMerge(buffer, buffer, catalog, 32);
}

//---------------------------------------------------------------------------------
void debugIO(u16 port, u8 val, const char *message) {
	char debugString[32];

	debugString[0] = 0;
	strlcat(debugString, message, sizeof(debugString));
	short2HexStr(&debugString[strlen(debugString)], port);
	strlcat(debugString, " val:", sizeof(debugString));
	char2HexStr(&debugString[strlen(debugString)], val);
	debugOutput(debugString);
}
//---------------------------------------------------------------------------------
void debugIOUnimplR(u16 port) {
	debugIO(port, 0, "Unimpl R port:");
}
void debugIOUnimplW(u8 val, u16 port) {
	debugIO(port, val, "Unimpl W port:");
}
void debugIOUnmappedR(u16 port) {
	debugIO(port, 0, "Unmapped R port:");
}
void debugIOUnmappedW(u8 val, u16 port) {
	debugIO(port, val, "Unmapped W port:");
}
void debugROMW(u8 val, u16 adr) {
	debugIO(adr, val, "Rom W:");
}
void debugSerialOutW(u8 val) {
	if (val < 0x80) {
		serialOut[serialPos++] = val;
		if (serialPos >= 32 || val == 0) {
			serialPos = 0;
			debugOutput(serialOut);
		}
	}
}
void debugDivideError() {
	debugOutput("Divide Error.");
}
void debugUndefinedInstruction() {
	debugOutput("Undefined Instruction.");
}
void debugCrashInstruction() {
	debugOutput("CPU Crash! (0xF1)");
}

void stepFrameUI() {
	stepFrame();
	setupMenuPalette();
}
//---------------------------------------------------------------------------------
/// Swap A & B buttons
void swapABSet() {
	joyCfg ^= 0x400;
}
const char *getSwapABText() {
	return autoTxt[(joyCfg>>10)&1];
}

/// Change gamma (brightness)
void gammaChange() {
	gammaSet();
	paletteInit(gGammaValue, gContrastValue, 0);
	monoPalInit(gGammaValue, gContrastValue, 0);
	paletteTxAll();					// Make new palette visible
//	setupEmuBorderPalette();
	setupMenuPalette();
}

/// Change contrast
void contrastSet() {
	gContrastValue++;
	if (gContrastValue > 4) gContrastValue = 0;
	paletteInit(gGammaValue, gContrastValue, 0);
	monoPalInit(gGammaValue, gContrastValue, 0);
	paletteTxAll();					// Make new palette visible
//	setupEmuBorderPalette();
	setupMenuPalette();
	settingsChanged = true;
}
const char *getContrastText() {
	return brighTxt[gContrastValue];
}

/// Turn on/off rendering of foreground
void fgrLayerSet() {
	gGfxMask ^= 0x02;
}
const char *getFgrLayerText() {
	return autoTxt[(gGfxMask>>1)&1];
}
/// Turn on/off rendering of background
void bgrLayerSet() {
	gGfxMask ^= 0x01;
}
const char *getBgrLayerText() {
	return autoTxt[gGfxMask&1];
}
/// Turn on/off rendering of sprites
void sprLayerSet() {
	gGfxMask ^= 0x10;
}
const char *getSprLayerText() {
	return autoTxt[(gGfxMask>>4)&1];
}
/// Turn on/off windows
void winLayerSet() {
	gGfxMask ^= 0x20;
}
const char *getWinLayerText() {
	return autoTxt[(gGfxMask>>5)&1];
}

void paletteChange() {
	gPaletteBank++;
	if (gPaletteBank > 7) {
		gPaletteBank = 0;
	}
	monoPalInit(gGammaValue, gContrastValue, 0);
	paletteTxAll();
	setupMenuPalette();
	settingsChanged = true;
}
const char *getPaletteText() {
	return palTxt[gPaletteBank];
}

void borderSet() {
	gBorderEnable ^= 0x01;
	setupEmuBorderPalette();
	setupMenuPalette();
}
const char *getBorderText() {
	return bordTxt[gBorderEnable];
}

void languageSet() {
	gLang ^= 0x01;
}

void machineSet() {
	gMachineSet++;
	if (gMachineSet >= HW_SELECT_END) {
		gMachineSet = 0;
	}
}
const char *getMachineText() {
	return machTxt[gMachineSet];
}

void speedHackSet() {
	emuSettings ^= ALLOW_SPEED_HACKS;
	emuSettings &= ~HALF_CPU_SPEED;
	hacksInit();
}
const char *getSpeedHackText() {
	return autoTxt[(emuSettings & ALLOW_SPEED_HACKS)>>17];
}
void cpuHalfSet() {
	emuSettings ^= HALF_CPU_SPEED;
	emuSettings &= ~ALLOW_SPEED_HACKS;
//	tweakCpuSpeed(emuSettings & HALF_CPU_SPEED);
}
const char *getCPUHalfText() {
	return autoTxt[(emuSettings & HALF_CPU_SPEED)>>16];
}

void headphonesSet() {
	emuSettings ^= ENABLE_HEADPHONES;
	setHeadphones(emuSettings & ENABLE_HEADPHONES);
}
const char *getHeadphonesText() {
	return autoTxt[(emuSettings & ENABLE_HEADPHONES)>>18];
}

void soundSet() {
	soundEnableSet();
	soundMode = (emuSettings & SOUND_ENABLE)>>10;
	soundInit();
}

void batteryChange() {
	batteryLevel = 15000;				// 0x15000 for 24h battery?
}
