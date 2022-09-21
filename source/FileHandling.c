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
#include "RomHeader.h"
#include "GUI.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"

static u32 headerId = SMSID;
static const u8 *romData;
static int romCount = 0;
int selectedGame = 0;
ConfigData cfg;

// Set text_start (before moving the rom)
extern u8 __rom_end__[];

//---------------------------------------------------------------------------------
const u8 *findRomHeader(const u8 *base, u32 headerId)
{
	// Look up to 256 bytes later for a ROM header
	const u32 *p=(u32*)base;
	
	int i;
	for (i=0; i<64; i++) {
		if (*p == headerId) {
			return ((u8*)p)-48;
		}
		else {
			p++;
		}
	}
	return NULL;
}

void initLocateRoms(u32 inHeaderId) {
	romData = __rom_end__;
	headerId = inHeaderId;
	const u8 *p = romData;
	romCount = 0;
	while (p && *(u32*)(p+sizeof(romheader)) == headerId) {
		// Count roms
		p += *(u32*)(p+32)+sizeof(romheader);
		p = findRomHeader(p, headerId);
		romCount++;
	}
}

// Return ptr to Nth ROM (including rominfo struct)
const u8 *findrom(int n)
{
	const u8 *p = findRomHeader(romData, headerId);
	while(p && n--)
	{
		p += *(u32*)(p+32)+sizeof(romheader);
		p = findRomHeader(p, headerId);
	}
	return p;
}
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
/*
void loadState(void) {
	u32 *statePtr;
//	FILE *file;
	char stateName[32];

	if (findFolder(folderName)) {
		return;
	}
	strlcpy(stateName, gameNames[selectedGame], sizeof(stateName));
	strlcat(stateName, ".sta", sizeof(stateName));
	int stateSize = getStateSize();
	if ( (file = fopen(stateName, "r")) ) {
		if ( (statePtr = malloc(stateSize)) ) {
			fread(statePtr, 1, stateSize, file);
			unpackState(statePtr);
			free(statePtr);
			infoOutput("Loaded state.");
		} else {
			infoOutput("Couldn't alloc mem for state.");
		}
		fclose(file);
	}
}

void saveState(void) {
	u32 *statePtr;
//	FILE *file;
	char stateName[32];

	if (findFolder(folderName)) {
		return;
	}
	strlcpy(stateName, gameNames[selectedGame], sizeof(stateName));
	strlcat(stateName, ".sta", sizeof(stateName));
	int stateSize = getStateSize();
	if ( (file = fopen(stateName, "w")) ) {
		if ( (statePtr = malloc(stateSize)) ) {
			packState(statePtr);
			fwrite(statePtr, 1, stateSize, file);
			free(statePtr);
			infoOutput("Saved state.");
		} else {
			infoOutput("Couldn't alloc mem for state.");
		}
		fclose(file);
	}
}
*/
//---------------------------------------------------------------------------------
bool loadGame() {
	if (loadRoms(selected, false)) {
		return true;
	}
	selectedGame = selected;
	loadRoms(selectedGame, true);
	setEmuSpeed(0);
	loadCart();
	if (emuSettings & AUTOLOAD_STATE) {
		loadState();
	}
	return false;
}

bool loadRoms(int game, bool doLoad) {
//	int i, j, count;
//	bool found;
//	u8 *romArea = ROM_Space;
//	FILE *file;

//	count = fileCount[game];
/*
	chdir("/");			// Stupid workaround.
	if (chdir(currentDir) == -1) {
		return true;
	}

	for (i=0; i<count; i++) {
		found = false;
		if ( (file = fopen(romFilenames[game][i], "r")) ) {
			if (doLoad) {
				fread(romArea, 1, romFilesizes[game][i], file);
				romArea += romFilesizes[game][i];
			}
			fclose(file);
			found = true;
		} else {
			for (j=0; j<GAMECOUNT; j++) {
				if ( !(findFileInZip(gameZipNames[j], romFilenames[game][i])) ) {
					if (doLoad) {
						loadFileInZip(romArea, gameZipNames[j], romFilenames[game][i], romFilesizes[game][i]);
						romArea += romFilesizes[game][i];
					}
					found = true;
					break;
				}
			}
		}
		if (!found) {
			infoOutput("Couldn't open file:");
			infoOutput(romFilenames[game][i]);
			return true;
		}
	}
*/
	return false;
}
