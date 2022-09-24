#include <gba.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/dir.h>

#include "FileHandling.h"
#include "Emubase.h"
#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "RomHeader.h"
#include "GUI.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"

int selectedGame = 0;
ConfigData cfg;

//---------------------------------------------------------------------------------
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
	gGammaValue = cfg.gammaValue;
	emuSettings  = cfg.emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	sleepTime    = cfg.sleepTime;
	joyCfg       = (joyCfg&~0x400)|((cfg.controller&1)<<10);
//	strlcpy(currentDir, cfg.currentPath, sizeof(currentDir));

	infoOutput("Settings loaded.");
	return 0;
}
void saveSettings() {
//	FILE *file;

	strcpy(cfg.magic,"cfg");
	cfg.gammaValue  = gGammaValue;
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

int loadNVRAM() {
	return 0;
}

void saveNVRAM() {
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
bool loadGame( const romheader *rh) {
	if (rh ) {
		gRomSize = rh->filesize;
		romSpacePtr = (const u8 *)rh + sizeof(romheader);
		selectedGame = selected;
		checkMachine();
		setEmuSpeed(0);
		loadCart();
		gameInserted = true;
		if ( emuSettings & AUTOLOAD_NVRAM ) {
			loadNVRAM();
		}
		if (emuSettings & AUTOLOAD_STATE) {
			loadState();
		}
		closeMenu();
		return false;
	}
	return true;
}

void selectGame() {
	pauseEmulation = true;
	setSelectedMenu(9);
	const romheader *rh = browseForFile();
	if ( loadGame(rh) ) {
		backOutOfMenu();
	}
}

void checkMachine() {
	if ( gMachineSet == HW_AUTO ) {
		if ( romSpacePtr[gRomSize - 9] != 0 ) {
			gMachine = HW_WONDERSWANCOLOR;
		}
//		else if ( strstr(fileExt, ".pc2") ) {
//			gMachine = HW_POCKETCHALLENGEV2;
//		}
		else {
			gMachine = HW_WONDERSWAN;
		}
	}
	else {
		gMachine = gMachineSet;
	}
	setupEmuBackground();
}
