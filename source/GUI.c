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
#include "io.h"
#include "cpu.h"
#include "ARMV30MZ/Version.h"
#include "Sphinx/Version.h"

#define EMUVERSION "V0.6.5 2024-05-05"

#define HALF_CPU_SPEED		(1<<16)
#define ALLOW_SPEED_HACKS	(1<<17)
#define ENABLE_HEADPHONES	(1<<18)

void hacksInit(void);

static void paletteChange(void);
static void machineSet(void);
static void batteryChange(void);
static void headphonesSet(void);
static void speedHackSet(void);
static void cpuHalfSet(void);
static void languageSet(void);
static void stepFrame(void);

static void uiDebug(void);
static void uiMachine(void);
static void updateGameInfo(char *buffer);

const fptr fnMain[] = {nullUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI};

const fptr fnList0[] = {uiDummy};
const fptr fnList1[] = {ui2, ui3, ui4, ui5, ui6, ui7, ui8, gbaSleep, resetGame, ui10};
const fptr fnList2[] = {selectGame, loadState, saveState, saveSettings, resetGame};
const fptr fnList3[] = {autoBSet, autoASet, swapABSet};
const fptr fnList4[] = {gammaSet, contrastSet, paletteChange};
const fptr fnList5[] = {speedSet, autoStateSet, autoSettingsSet, autoPauseGameSet, ewramSet, sleepSet};
const fptr fnList6[] = {machineSet, batteryChange, clearIntEeproms, headphonesSet, speedHackSet, cpuHalfSet /*languageSet*/};
const fptr fnList7[] = {debugTextSet, fgrLayerSet, bgrLayerSet, sprLayerSet, stepFrame};
const fptr fnList8[] = {uiDummy};
const fptr fnList9[] = {quickSelectGame};
const fptr fnList10[] = {exitEmulator, backOutOfMenu};
const fptr *const fnListX[] = {fnList0, fnList1, fnList2, fnList3, fnList4, fnList5, fnList6, fnList7, fnList8, fnList9, fnList10};
u8 menuXItems[] = {ARRSIZE(fnList0), ARRSIZE(fnList1), ARRSIZE(fnList2), ARRSIZE(fnList3), ARRSIZE(fnList4), ARRSIZE(fnList5), ARRSIZE(fnList6), ARRSIZE(fnList7), ARRSIZE(fnList8), ARRSIZE(fnList9), ARRSIZE(fnList10)};
const fptr drawUIX[] = {uiNullNormal, uiMainMenu, uiFile, uiController, uiDisplay, uiSettings, uiMachine, uiDebug, uiAbout, uiLoadGame};

u8 gGammaValue = 0;
u8 gContrastValue = 3;
u8 gBorderEnable = 1;
u8 serialPos = 0;
char serialOut[32];

const char *const autoTxt[]  = {"Off", "On", "With R"};
const char *const speedTxt[] = {"Normal", "200%", "Max", "50%"};
const char *const brighTxt[] = {"I", "II", "III", "IIII", "IIIII"};
const char *const sleepTxt[] = {"5min", "10min", "30min", "Off"};

const char *const machTxt[]  = {"Auto", "WonderSwan", "WonderSwan Color", "SwanCrystal", "Pocket Challenge V2"};
const char *const bordTxt[]  = {"Black", "Border Color", "None"};
const char *const palTxt[]   = {"Classic", "Black & White", "Red", "Green", "Blue", "Green-Blue", "Blue-Green", "Puyo Puyo Tsu"};
const char *const langTxt[]  = {"Japanese", "English"};

/// This is called at the start of the emulator
void setupGUI() {
	emuSettings = AUTOPAUSE_EMULATION | AUTOLOAD_NVRAM | ALLOW_SPEED_HACKS;
//	keysSetRepeat(25, 4);	// Delay, repeat.
	menuXItems[1] = ARRSIZE(fnList1) - (enableExit?0:1);
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

void uiFile() {
	setupSubMenu("File Handling");
	drawMenuItem("Load Game->");
	drawMenuItem("Load State");
	drawMenuItem("Save State");
	drawMenuItem("Save Settings");
	drawMenuItem("Reset Game");
}

void uiMainMenu() {
	setupSubMenu("Main Menu");
	drawMenuItem("File->");
	drawMenuItem("Controller->");
	drawMenuItem("Display->");
	drawMenuItem("Settings->");
	drawMenuItem("Machine->");
	drawMenuItem("Debug->");
	drawMenuItem("About->");
	drawMenuItem("Sleep");
	drawMenuItem("Reset Console");
	if (enableExit) {
		drawMenuItem("Quit Emulator");
	}
}

void uiAbout() {
	setupSubMenu("About");
	char gameInfoString[32];
	updateGameInfo(gameInfoString);
	drawText("B:        WS B Button", 3);
	drawText("A:        WS A Button", 4);
	drawText("Start:    WS Start Button", 5);
	drawText("Select:   WS Sound Button", 6);
	drawText("DPad:     WS X1-X4", 7);

	drawText(gameInfoString, 9);

	drawText("SwanGBA    " EMUVERSION, 17);
	drawText("Sphinx     " SPHINXVERSION, 18);
	drawText("ARMV30MZ   " ARMV30MZVERSION, 19);
}

void uiController() {
	setupSubMenu("Controller Settings");
	drawSubItem("B Autofire: ", autoTxt[autoB]);
	drawSubItem("A Autofire: ", autoTxt[autoA]);
	drawSubItem("Swap A-B:   ", autoTxt[(joyCfg>>10)&1]);
}

void uiDisplay() {
	setupSubMenu("Display Settings");
	drawSubItem("Gamma: ", brighTxt[gGammaValue]);
	drawSubItem("Contrast:", brighTxt[gContrastValue]);
	drawSubItem("B&W Palette: ", palTxt[gPaletteBank]);
}

static void uiMachine() {
	setupSubMenu("Machine Settings");
	drawSubItem("Machine: ",machTxt[gMachineSet]);
	drawMenuItem("Change Batteries");
	drawMenuItem("Clear Internal EEPROM");
	drawSubItem("Headphones:", autoTxt[(emuSettings&ENABLE_HEADPHONES)>>18]);
	drawSubItem("Cpu Speed Hacks: ",autoTxt[(emuSettings&ALLOW_SPEED_HACKS)>>17]);
	drawSubItem("Half Cpu Speed: ",autoTxt[(emuSettings&HALF_CPU_SPEED)>>16]);
//	drawSubItem("Language: ",langTxt[gLang]);
}

void uiSettings() {
	setupSubMenu("Other Settings");
	drawSubItem("Speed: ", speedTxt[(emuSettings>>6)&3]);
	drawSubItem("Autoload State: ", autoTxt[(emuSettings>>2)&1]);
	drawSubItem("Autosave Settings: ", autoTxt[(emuSettings>>1)&1]);
	drawSubItem("Autopause Game: ", autoTxt[emuSettings&1]);
	drawSubItem("EWRAM Overclock: ", autoTxt[ewram&1]);
	drawSubItem("Autosleep: ", sleepTxt[(emuSettings>>8)&3]);
}

void uiDebug() {
	setupSubMenu("Debug");
	drawSubItem("Debug Output: ", autoTxt[gDebugSet&1]);
	drawSubItem("Disable Foreground: ", autoTxt[(gGfxMask>>1)&1]);
	drawSubItem("Disable Background: ", autoTxt[gGfxMask&1]);
	drawSubItem("Disable Sprites: ", autoTxt[(gGfxMask>>4)&1]);
	drawSubItem("Step Frame ", NULL);
}

void uiLoadGame() {
	setupSubMenu("Load game");
}

void nullUINormal(int key) {
}

void nullUIDebug(int key) {
}

void resetGame() {
	powerIsOn = true;
	loadCart();
	setupEmuBackground();
}

void updateGameInfo(char *buffer) {
	char catalog[8];
	char2HexStr(catalog, gGameHeader->gameId);
	strlMerge(buffer, "Game Id, Rev #: 0x", catalog, 32);
	strlMerge(buffer, buffer, " 0x", 32);
	char2HexStr(catalog, gGameHeader->gameRev);
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

void stepFrame() {
	runFrame();
	setupMenuPalette();
}
//---------------------------------------------------------------------------------
/// Swap A & B buttons
void swapABSet() {
	joyCfg ^= 0x400;
}

/// Change gamma (brightness)
void gammaSet() {
	gGammaValue++;
	if (gGammaValue > 4) gGammaValue=0;
	paletteInit(gGammaValue, gContrastValue);
	monoPalInit(gGammaValue, gContrastValue);
	paletteTxAll();					// Make new palette visible
	setupMenuPalette();
	settingsChanged = true;
}

/// Change contrast
void contrastSet() {
	gContrastValue++;
	if (gContrastValue > 4) gContrastValue = 0;
	paletteInit(gGammaValue, gContrastValue);
	monoPalInit(gGammaValue, gContrastValue);
	paletteTxAll();					// Make new palette visible
//	setupEmuBorderPalette();
	setupMenuPalette();
	settingsChanged = true;
}

/// Turn on/off rendering of foreground
void fgrLayerSet() {
	gGfxMask ^= 0x02;
}
/// Turn on/off rendering of background
void bgrLayerSet() {
	gGfxMask ^= 0x01;
}
/// Turn on/off rendering of sprites
void sprLayerSet() {
	gGfxMask ^= 0x10;
}

void paletteChange() {
	gPaletteBank++;
	if (gPaletteBank > 7) {
		gPaletteBank = 0;
	}
	monoPalInit(gGammaValue, gContrastValue);
	paletteTxAll();
	setupMenuPalette();
	settingsChanged = true;
}

void borderSet() {
	gBorderEnable ^= 0x01;
//	setupEmuBorderPalette();
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

void speedHackSet() {
	emuSettings ^= ALLOW_SPEED_HACKS;
	emuSettings &= ~HALF_CPU_SPEED;
	hacksInit();
}
void cpuHalfSet() {
	emuSettings ^= HALF_CPU_SPEED;
	emuSettings &= ~ALLOW_SPEED_HACKS;
//	tweakCpuSpeed(emuSettings & HALF_CPU_SPEED);
}

void headphonesSet() {
	emuSettings ^= ENABLE_HEADPHONES;
	setHeadphones(emuSettings & ENABLE_HEADPHONES);
}

void batteryChange() {
	batteryLevel = 15000;				// 0x15000 for 24h battery?
}
