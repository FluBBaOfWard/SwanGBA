#include <gba.h>
#include <string.h>

#include "FileHandling.h"
#include "Emubase.h"
#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "Shared/SRAMHandler.h"
#include "Shared/AsmExtra.h"
#include "Gui.h"
#include "Cart.h"
#include "Gfx.h"
#include "Sound.h"
#include "io.h"
#include "InternalEEPROM.h"
#include "WSCart/WSCart.h"

/// Used for GBA emulators or flash-carts to choose save type.
const char *const SRAM_TAG = "SRAM_Vnnn";

static const char *const wsEepromName = "wsinternal.eeprom";
static const char *const wscEepromName = "wscinternal.eeprom";
static const char *const scEepromName = "scinternal.eeprom";
static const char *const swanGBAName = "@ SwanGBA @";

char translateDSChar(u16 char16);

EWRAM_BSS int selectedGame = 0;
EWRAM_BSS ConfigData cfg;

//---------------------------------------------------------------------------------
void applyConfigData(void) {
	emuSettings   = cfg.emuSettings & ~EMUSPEED_MASK; // Clear speed setting.
	gBorderEnable = (cfg.config & 1) ^ 1;
	gPaletteBank  = cfg.palette;
	gGammaValue   = cfg.gammaValue & 0xF;
	gContrastValue = (cfg.gammaValue >> 4) & 0xF;
	soundMode     = (emuSettings & SOUND_ENABLE) >> 10;
	joyCfg        = (joyCfg & ~0x400) | ((cfg.controller & 1) << 10);
	gMachineSet   = (cfg.machine >> 1) & 0x7;
	setSleepValue(emuSettings & AUTOSLEEP_MASK);
}

void updateConfigData(void) {
	cfg.emuSettings = emuSettings & ~EMUSPEED_MASK; // Clear speed setting.
	cfg.config      = (gBorderEnable & 1) ^ 1;
	cfg.palette     = gPaletteBank;
	cfg.gammaValue  = (gGammaValue & 0xF) | (gContrastValue<<4);
	cfg.controller  = (joyCfg >> 10) & 1;
	cfg.machine     = (gMachineSet & 7) << 1;
}

void initSettings() {
	memset(&cfg, 0, sizeof(cfg));
	cfg.emuSettings = AUTOPAUSE_EMULATION | AUTOLOAD_NVRAM | AUTOSLEEP_OFF | ALLOW_SPEED_HACKS;
	cfg.gammaValue  = 0x20;
	cfg.birthYear[0] = 0x19;
	cfg.birthYear[1] = 0x99;
//	cfg.birthMonth = bin2BCD(PersonalData->birthMonth);
//	cfg.birthDay = bin2BCD(PersonalData->birthDay);
	settingsChanged = true;

	int i;
	for (i = 0; i < 11; i++) {
		u16 char16 = swanGBAName[i];
		cfg.name[i] = translateDSChar(char16);
	}
	cfg.name[i] = 0;

	applyConfigData();
}

char translateDSChar(u16 char16) {
	// Translate numbers.
	if (char16 > 0x2F && char16 < 0x3A) {
		return char16 - 0x2F;
	}
	// Translate normal chars.
	if ((char16 > 0x40 && char16 < 0x5B) || (char16 > 0x60 && char16 < 0x7B)) {
		return (char16 & 0x1F) + 10;
	}
	// Check for heart (♥︎).
	if (char16 == 0xE017 || char16 == 0x0040) {
		return 0x25;
	}
	// Check for note (♪).
	if (char16 == 0x266A) {
		return 0x26;
	}
	// Check for plus (+).
	if (char16 == 0x002B) {
		return 0x27;
	}
	// Check for minus/dash (-).
	if (char16 == 0x002D || char16 == 0x30FC) {
		return 0x28;
	}
	// Check for different question marks (?).
	if (char16 == 0x003F || char16 == 0xFF1F || char16 == 0xE011) {
		return 0x29;
	}
	// Check for different dots/full stop (.).
	if (char16 == 0x002E || char16 == 0x3002) {
		return 0x2A;
	}
	return 0; // Space
}

bool updateSettingsFromWS() {
	bool changed = false;
	IntEEPROM *intProm = (IntEEPROM *)intEeprom.memory;

	WSUserData *userData = &intProm->userData;
	if (cfg.birthYear[0] != userData->birthYear[0]
		|| cfg.birthYear[1] != userData->birthYear[1]) {
		cfg.birthYear[0] = userData->birthYear[0];
		cfg.birthYear[1] = userData->birthYear[1];
		changed = true;
	}
	if (cfg.birthMonth != userData->birthMonth) {
		cfg.birthMonth = userData->birthMonth;
		changed = true;
	}
	if (cfg.birthDay != userData->birthDay) {
		cfg.birthDay = userData->birthDay;
		changed = true;
	}
	if (cfg.sex != userData->sex) {
		cfg.sex = userData->sex;
		changed = true;
	}
	if (cfg.bloodType != userData->bloodType) {
		cfg.bloodType = userData->bloodType;
		changed = true;
	}
	if (memcmp(cfg.name, userData->name, 16) != 0) {
		memcpy(cfg.name, userData->name, 16);
		changed = true;
	}
	settingsChanged |= changed;
	return changed;
}

int loadSettings() {
	if (readFile((u8 *)&cfg, sizeof(cfg), BWSID)) {
		applyConfigData();
		settingsChanged = false;
		infoOutput("Settings loaded.");
		return 0;
	}
	else {
		updateConfigData();
		infoOutput("No settings file found.");
	}
	return 1;
}
void saveSettings() {
	updateConfigData();

	if (writeFile((u8 *)&cfg, sizeof(cfg), BWSID, "Config")) {
		settingsChanged = false;
		infoOutput("Settings saved.");
	}
	else {
		infoOutput("Could not save settings file.");
	}
	saveIntEeproms();
}

int loadNVRAM() {
	int saveSize = 0;
	void *nvMem = NULL;

	if (sramSize > 0) {
		saveSize = 0x8000;
		nvMem = cartSRAM;
	}
	else if (eepromSize > 0) {
		saveSize = eepromSize;
		nvMem = cartEepromMem;
	}
	else {
		return 0;
	}
	if (loadEmuSram(nvMem, saveSize)) {
		infoOutput("Loaded NVRAM.");
		return 1;
	}
	return 0;
}

int saveNVRAM() {
	int saveSize = 0;
	void *nvMem = NULL;

	if (sramSize > 0) {
		saveSize = 0x8000;
		nvMem = cartSRAM;
	}
	else if (eepromSize > 0) {
		saveSize = eepromSize;
		nvMem = cartEepromMem;
	}
	else {
		return 0;
	}
	if (saveEmuSram(nvMem, saveSize)) {
		infoOutput("Saved NVRAM.");
		return 1;
	}
	return 0;
}

int loadStateChk() {
	if (getStateSize() < 0x10000
		&& quickLoad()) {
		infoOutput("Loaded State.");
		return loadNVRAM();
	}
	return 0;
}
int saveStateChk() {
	if (getStateSize() < 0x10000
		&& quickSave()) {
		infoOutput("Saved State.");
		return saveNVRAM();
	}
	return 0;
}

void loadState(void) {
	loadStateChk();
}
void saveState(void) {
	saveStateChk();
}

//---------------------------------------------------------------------------------
int loadIntEeprom(const char *name, u8 *dest, int size, int id) {
	if (readFile(dest, size, id)) {
		infoOutput("Internal EEPROM loaded.");
		return 1;
	}

	infoOutput("Couldn't load EEPROM");
	return 0;
}

int saveIntEeprom(const char *name, const u8 *source, int size, int id) {
	if (writeFile(source, size, id, name)) {
		infoOutput("Internal EEPROM saved.");
		return 1;
	}

	infoOutput("Couldn't save EEPROM");
	return 0;
}

static void initIntEepromWS(IntEEPROM *intProm) {
	WSUserData *userData = &intProm->userData;
	memcpy(userData->name, cfg.name, 16);
	userData->birthYear[0] = cfg.birthYear[0];
	userData->birthYear[1] = cfg.birthYear[1];
	userData->birthMonth = cfg.birthMonth;
	userData->birthDay = cfg.birthDay;
	userData->sex = cfg.sex;
	userData->bloodType = cfg.bloodType;
}
static void initIntEepromWSC(IntEEPROM *intProm) {
	initIntEepromWS(intProm);
	intProm->splashData.consoleFlags = 3;
}

static void initIntEepromSC(IntEEPROM *intProm) {
	initIntEepromWSC(intProm);
	WSBootSplash *splashData = &intProm->splashData;
	splashData->crystalLCD70 = 0xd0;
	splashData->crystalLCD71 = 0x77;
	splashData->crystalLCD72 = 0xf7;
	splashData->crystalLCD73 = 0x06;
	splashData->crystalLCD74 = 0xe2;
	splashData->crystalLCD75 = 0x0a;
	splashData->crystalLCD76 = 0xea;
	splashData->crystalLCD77 = 0xee;
}

static void clearIntEepromWS() {
	memset(wsEepromMem, 0, sizeof(wsEepromMem));
	initIntEepromWS((IntEEPROM *)wsEepromMem);
}
static void clearIntEepromWSC() {
	memset(wscEepromMem, 0, sizeof(wscEepromMem));
	initIntEepromWSC((IntEEPROM *)wscEepromMem);
}
static void clearIntEepromSC() {
	memset(scEepromMem, 0, sizeof(scEepromMem));
	initIntEepromSC((IntEEPROM *)scEepromMem);
}

int loadIntEeproms() {
	clearIntEepromWS();
	clearIntEepromWSC();
	clearIntEepromSC();
	int status;
	status = loadIntEeprom(wsEepromName, wsEepromMem, sizeof(wsEepromMem), 12);
	status |= loadIntEeprom(wscEepromName, wscEepromMem, sizeof(wscEepromMem), 13);
	status |= loadIntEeprom(scEepromName, scEepromMem, sizeof(scEepromMem), 14);
	return status;
}

int saveIntEeproms() {
	int status = 0;
	switch (gSOC) {
		case SOC_ASWAN:
			status = saveIntEeprom(wsEepromName, wsEepromMem, sizeof(wsEepromMem), 12);
			break;
		case SOC_SPHINX:
			status = saveIntEeprom(wscEepromName, wscEepromMem, sizeof(wscEepromMem), 13);
			break;
		case SOC_SPHINX2:
			status = saveIntEeprom(scEepromName, scEepromMem, sizeof(scEepromMem), 14);
			break;
		}
	return status;
}

void clearIntEeproms() {
	switch (gSOC) {
		case SOC_ASWAN:
			clearIntEepromWS();
			break;
		case SOC_SPHINX:
			clearIntEepromWSC();
			break;
		case SOC_SPHINX2:
			clearIntEepromSC();
			break;
	}
}

//---------------------------------------------------------------------------------
void loadBioses(void) {
	const RomHeader *bh;
	int n = 0;
	g_BIOSBASE_COLOR = NULL;
	g_BIOSBASE_CRYSTAL = NULL;
	g_BIOSBASE_BNW = NULL;
	while ((bh = findBios(n++))) {
		if (bh->flags & 0x4) {
			g_BIOSBASE_COLOR = bh->romData;
		}
		else if (bh->flags & 0x8) {
			g_BIOSBASE_CRYSTAL = bh->romData;
		}
		else if ((bh->flags & 0xC) == 0) {
			g_BIOSBASE_BNW = bh->romData;
		}
	}
}

bool loadGame(const RomHeader *rh) {
	if (rh) {
		return loadROM(rh->romData, rh->filesize, rh->flags);
	}
	return true;
}

bool loadROM(const u8 *rom, int size, int emuflags) {
	selectedGame = selected;
	gRomSize = size;
	romSpacePtr = rom;
	checkMachine(emuflags);
	setEmuSpeed(0);
	loadCart(emuflags);
	gameInserted = true;
	int loadedSRAM = 0;
	if (emuSettings & AUTOLOAD_STATE) {
		loadedSRAM = loadStateChk();
	}
	if ((emuSettings & AUTOLOAD_NVRAM)
		&& loadedSRAM == 0) {
		loadNVRAM();
	}
	powerIsOn = true;
	closeMenu();
	return false;
}

void selectGame() {
	pauseEmulation = true;
	ui9();
	const RomHeader *rh = browseForFile();
	if (loadGame(rh)) {
		backOutOfMenu();
	}
}

void viewNVRAM() {
	pauseEmulation = true;
	ui12();
	skipScroll();
	manageNVRAM();
	backOutOfMenu();
}

void viewSStates() {
	pauseEmulation = true;
	ui13();
	skipScroll();
	loadStateMenu();
	backOutOfMenu();
}

void checkMachine(int emuFlags) {
	if (gMachineSet == HW_AUTO) {
		if (romSpacePtr[gRomSize - 9] != 0 || emuFlags & 0xC) {
			gMachine = HW_WONDERSWANCOLOR;
			if (emuFlags & 0x8) {
				gMachine = HW_SWANCRYSTAL;
			}
		}
		else if (emuFlags & 2) {
			gMachine = HW_POCKETCHALLENGEV2;
		}
		else {
			gMachine = HW_WONDERSWAN;
		}
	}
	else {
		gMachine = gMachineSet;
	}
	setupEmuBackground();
}
