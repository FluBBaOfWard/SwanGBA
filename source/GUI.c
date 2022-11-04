#include <gba.h>
#include <string.h>

#include "Gui.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Main.h"
#include "WonderSwan.h"
#include "FileHandling.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"
#include "cpu.h"
#include "ARMV30MZ/Version.h"
#include "Sphinx/Version.h"

#define EMUVERSION "V0.4.2 2022-11-04"

#define HALF_CPU_SPEED		(1<<16)
#define ALLOW_SPEED_HACKS	(1<<17)

void hacksInit(void);

static void paletteChange(void);
static void languageSet(void);
static void machineSet(void);
static void batteryChange(void);
static void speedHackSet(void);
static void cpuHalfSet(void);

static void uiDebug(void);
static void uiMachine(void);
static void updateGameInfo(void);

const fptr fnMain[] = {nullUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI};

const fptr fnList0[] = {uiDummy};
const fptr fnList1[] = {ui2, ui3, ui4, ui5, ui6, ui7, ui8, gbaSleep, resetGame, ui10};
const fptr fnList2[] = {selectGame, loadState, saveState, saveSettings, resetGame};
const fptr fnList3[] = {autoBSet, autoASet, swapABSet};
const fptr fnList4[] = {gammaSet, paletteChange};
const fptr fnList5[] = {speedSet, autoStateSet, autoSettingsSet, autoPauseGameSet, ewramSet, sleepSet};
const fptr fnList6[] = {languageSet, machineSet, batteryChange, speedHackSet, cpuHalfSet};
const fptr fnList7[] = {debugTextSet, fgrLayerSet, bgrLayerSet, sprLayerSet, stepFrame};
const fptr fnList8[] = {uiDummy};
const fptr fnList9[] = {quickSelectGame};
const fptr fnList10[] = {exitEmulator, backOutOfMenu};
const fptr *const fnListX[] = {fnList0, fnList1, fnList2, fnList3, fnList4, fnList5, fnList6, fnList7, fnList8, fnList9, fnList10};
u8 menuXItems[] = {ARRSIZE(fnList0), ARRSIZE(fnList1), ARRSIZE(fnList2), ARRSIZE(fnList3), ARRSIZE(fnList4), ARRSIZE(fnList5), ARRSIZE(fnList6), ARRSIZE(fnList7), ARRSIZE(fnList8), ARRSIZE(fnList9), ARRSIZE(fnList10)};
const fptr drawUIX[] = {uiNullNormal, uiMainMenu, uiFile, uiController, uiDisplay, uiSettings, uiMachine, uiDebug, uiAbout, uiLoadGame};

u8 gGammaValue = 0;
char gameInfoString[32];

const char *const autoTxt[]  = {"Off", "On", "With R"};
const char *const speedTxt[] = {"Normal", "200%", "Max", "50%"};
const char *const sleepTxt[] = {"5min", "10min", "30min", "Off"};
const char *const brighTxt[] = {"I", "II", "III", "IIII", "IIIII"};
const char *const dispTxt[]  = {"Unscaled", "Scaled"};
const char *const flickTxt[] = {"No Flicker", "Flicker"};

const char *const machTxt[]  = {"Auto", "WonderSwan", "WonderSwan Color", "SwanCrystal", "Pocket Challenge V2"};
const char *const bordTxt[]  = {"Black", "Border Color", "None"};
const char *const palTxt[]   = {"Black & White", "Red", "Green", "Blue", "Classic"};
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
	setupBorderPalette();
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
	updateGameInfo();
	drawText("B:        WS B Button", 3);
	drawText("A:        WS A Button", 4);
	drawText("Start:    WS Start Button", 5);
	drawText("Select:   Sound Button", 6);
	drawText("DPad:     Joystick", 7);

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
	drawSubItem("B&W Palette: ", palTxt[gPaletteBank]);
}

static void uiMachine() {
	setupSubMenu("Machine Settings");
	drawSubItem("Language: ",langTxt[gLang]);
	drawSubItem("Machine: ",machTxt[gMachineSet]);
	drawMenuItem("Change Batteries");
	drawSubItem("Cpu speed hacks: ",autoTxt[(emuSettings&ALLOW_SPEED_HACKS)>>17]);
	drawSubItem("Half cpu speed: ",autoTxt[(emuSettings&HALF_CPU_SPEED)>>16]);
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
	loadCart();
	setupEmuBackground();
}

void updateGameInfo() {
	char catalog[8];
//	char2HexStr(catalog, gGameID);
	strlMerge(gameInfoString, " Game #: 0x", catalog, sizeof(gameInfoString));
}
//---------------------------------------------------------------------------------
void debugIO(u8 port, u8 val, const char *message) {
	char debugString[32];

	debugString[0] = 0;
	strlcat(debugString, message, sizeof(debugString));
	char2HexStr(&debugString[strlen(debugString)], port);
	strlcat(debugString, " val:", sizeof(debugString));
	char2HexStr(&debugString[strlen(debugString)], val);
	debugOutput(debugString);
}
//---------------------------------------------------------------------------------
void debugIOUnimplR(u8 port) {
	debugIO(port, 0, "Unimpl R port:");
}
void debugIOUnimplW(u8 port, u8 val) {
	debugIO(port, val, "Unimpl W port:");
}
void debugIOUnmappedR(u8 port) {
	debugIO(port, 0, "Unmapped R port:");
}
void debugIOUnmappedW(u8 port, u8 val) {
	debugIO(port, val, "Unmapped W port:");
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
//---------------------------------------------------------------------------------
/// Swap A & B buttons
void swapABSet() {
	joyCfg ^= 0x400;
}

/// Change gamma (brightness)
void gammaSet() {
	gGammaValue++;
	if (gGammaValue > 4) gGammaValue=0;
	paletteInit(gGammaValue);
	paletteTxAll();					// Make new palette visible
	setupMenuPalette();
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
	if (gPaletteBank > 4) {
		gPaletteBank = 0;
	}
	monoPalInit();
	paletteTxAll();
	settingsChanged = true;
}
/*
void borderSet() {
	bcolor++;
	if (bcolor > 2) {
		bcolor = 0;
	}
	makeborder();
}
*/
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

void batteryChange() {
	batteryLevel = 0xFFFF;				// 0xFFFF for 2 days battery?
}
