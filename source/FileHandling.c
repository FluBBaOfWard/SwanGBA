#include <gba.h>
#include <string.h>

#include "FileHandling.h"
#include "Emubase.h"
#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "Shared/AsmExtra.h"
#include "Gui.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"
#include "InternalEEPROM.h"

static const char *const swanGBAName = "@ SwanGBA @";

char translateDSChar(u16 char16);

EWRAM_BSS int selectedGame = 0;
EWRAM_BSS ConfigData cfg;

//---------------------------------------------------------------------------------
int initSettings() {
	cfg.config = 0;
	cfg.palette = 0;
	cfg.gammaValue = 0x20;
	cfg.emuSettings = AUTOPAUSE_EMULATION | AUTOLOAD_NVRAM | ALLOW_SPEED_HACKS;
	cfg.sleepTime = 60*60*5;
	cfg.controller = 0;					// Don't swap A/B
	cfg.birthYear[0] = 0x19;
	cfg.birthYear[1] = 0x99;
//	cfg.birthMonth = bin2BCD(PersonalData->birthMonth);
//	cfg.birthDay = bin2BCD(PersonalData->birthDay);
	cfg.sex = 0;
	cfg.bloodType = 0;
//	cfg.language = (PersonalData->language == 0) ? 0 : 1;

	int i;
	for (i = 0; i < 11; i++) {
		s16 char16 = swanGBAName[i];
		cfg.name[i] = translateDSChar(char16);
	}
	cfg.name[i] = 0;
	return 0;
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
	}
	settingsChanged |= changed;
	return changed;
}

int loadSettings() {
//	FILE *file;
/*
	if (findFolder(folderName)) {
		return 1;
	}
	if ( (file = fopen(settingName, "r")) ) {
		fread(&cfg, 1, sizeof(configdata), file);
		fclose(file);
		if (!strstr(cfg.magic,"cfg")) {
			infoOutput("Error in settings file.");
			return 1;
		}
	} else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
		return 1;
	}
*/
	gBorderEnable = (cfg.config & 1) ^ 1;
	gPaletteBank  = cfg.palette;
	gGammaValue   = cfg.gammaValue & 0xF;
	gContrastValue = (cfg.gammaValue>>4) & 0xF;
	emuSettings = cfg.emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	sleepTime   = cfg.sleepTime;
	joyCfg      = (joyCfg&~0x400)|((cfg.controller&1)<<10);
//	strlcpy(currentDir, cfg.currentPath, sizeof(currentDir));

	infoOutput("Settings loaded.");
	return 0;
}
void saveSettings() {
//	FILE *file;

	strcpy(cfg.magic,"cfg");
	cfg.gammaValue  = (gGammaValue & 0xF) | (gContrastValue<<4);
	cfg.emuSettings = emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	cfg.sleepTime   = sleepTime;
	cfg.controller  = (joyCfg>>10)&1;
//	strlcpy(cfg.currentPath, currentDir, sizeof(currentDir));
/*
	if (findFolder(folderName)) {
		return;
	}
	if ( (file = fopen(settingName, "w")) ) {
		fwrite(&cfg, 1, sizeof(configdata), file);
		fclose(file);
		infoOutput("Settings saved.");
	} else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
	}*/
	infoOutput("Settings saved.");
}

void loadNVRAM() {
	int saveSize = 0;
	void *nvMem = NULL;

	if (sramSize > 0) {
		saveSize = sizeof(wsSRAM);
		nvMem = wsSRAM;
	}
	else if (eepromSize > 0) {
		saveSize = eepromSize;
		nvMem = extEepromMem;
	}
	else {
		return;
	}
	bytecopy_(nvMem, (u8 *)SRAM, saveSize);
	infoOutput("Loaded NVRAM.");
}

void saveNVRAM() {
	int saveSize = 0;
	void *nvMem = NULL;

	if (sramSize > 0) {
		saveSize = sizeof(wsSRAM);
		nvMem = wsSRAM;
	}
	else if (eepromSize > 0) {
		saveSize = eepromSize;
		nvMem = extEepromMem;
	}
	else {
		return;
	}
	bytecopy_((u8 *)SRAM, nvMem, saveSize);
	infoOutput("Saved NVRAM.");
}

void loadState(void) {
//	unpackState(testState);
	infoOutput("Loaded state.");
}
void saveState(void) {
//	packState(testState);
	infoOutput("Saved state.");
}

//---------------------------------------------------------------------------------
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
	initIntEepromWSC((IntEEPROM *)scEepromMem);
}

int loadIntEeproms() {
	int status = 0;
	clearIntEepromWS();
	clearIntEepromWSC();
	clearIntEepromSC();
//	if (!findFolder(folderName)) {
//		status = loadIntEeprom(wsEepromName, wsEepromMem, sizeof(wsEepromMem));
//		status |= loadIntEeprom(wscEepromName, wscEepromMem, sizeof(wscEepromMem));
//		status |= loadIntEeprom(scEepromName, scEepromMem, sizeof(scEepromMem));
//	}
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
			g_BIOSBASE_COLOR = (const u8 *)bh + sizeof(RomHeader);
		}
		else if (bh->flags & 0x8) {
			g_BIOSBASE_CRYSTAL = (const u8 *)bh + sizeof(RomHeader);
		}
		else if ((bh->flags & 0xC) == 0) {
			g_BIOSBASE_BNW = (const u8 *)bh + sizeof(RomHeader);
		}
	}
}

bool loadGame(const RomHeader *rh) {
	if (rh) {
		gRomSize = rh->filesize;
		romSpacePtr = (const u8 *)rh + sizeof(RomHeader);
		selectedGame = selected;
		checkMachine(rh);
		setEmuSpeed(0);
		loadCart();
		gameInserted = true;
		if (emuSettings & AUTOLOAD_NVRAM) {
			loadNVRAM();
		}
		if (emuSettings & AUTOLOAD_STATE) {
			loadState();
		}
		powerIsOn = true;
		closeMenu();
		return false;
	}
	return true;
}

void selectGame() {
	pauseEmulation = true;
	ui9();
	const RomHeader *rh = browseForFile();
	if (loadGame(rh)) {
		backOutOfMenu();
	}
}

void checkMachine(const RomHeader *rh) {
	if (gMachineSet == HW_AUTO) {
		if (romSpacePtr[gRomSize - 9] != 0 || rh->flags & 0xC) {
			gMachine = HW_WONDERSWANCOLOR;
			if (rh->flags & 0x4) {
				gMachine = HW_SWANCRYSTAL;
			}
		}
		else if (rh->flags & 2) {
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
