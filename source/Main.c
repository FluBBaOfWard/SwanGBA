#include <gba.h>
#include <string.h>

#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/FileHelper.h"
#include "Shared/AsmExtra.h"
#include "Gui.h"
#include "FileHandling.h"
#include "EmuFont.h"
#include "WonderSwan.h"
#include "Cart.h"
#include "cpu.h"
#include "Gfx.h"
#include "io.h"
#include "Sound.h"

static void checkTimeOut(void);
static void setupGraphics(void);

bool powerIsOn = false;
bool gameInserted = false;
static int sleepTimer = 60*60*5;	// 5 min

u16 *menuMap;

static const u8 guiPalette[] = {
	0x00,0x00,0x00, 0x00,0x00,0x00, 0x50,0x78,0x78, 0x60,0x90,0x90, 0x78,0xB0,0xB0, 0x88,0xC8,0xC8, 0x90,0xE0,0xE0, 0xA0,0xF0,0xF0,
	0xB8,0xF8,0xF8, 0xEF,0xFF,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,
	0x00,0x00,0xC0, 0x00,0x00,0x00, 0x81,0x81,0x81, 0x93,0x93,0x93, 0xA5,0xA5,0xA5, 0xB7,0xB7,0xB7, 0xC9,0xC9,0xC9, 0xDB,0xDB,0xDB,
	0xED,0xED,0xED, 0xFF,0xFF,0xFF, 0x00,0x00,0xC0, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,

	0x00,0x00,0x00, 0x00,0x00,0x00, 0x21,0x21,0x21, 0x33,0x33,0x33, 0x45,0x45,0x45, 0x47,0x47,0x47, 0x59,0x59,0x59, 0x6B,0x6B,0x6B,
	0x7D,0x7D,0x7D, 0x8F,0x8F,0x8F, 0x20,0x20,0xE0, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,

	0x00,0x00,0x00, 0x00,0x00,0x00, 0x81,0x81,0x81, 0x93,0x93,0x93, 0xA5,0xA5,0xA5, 0xB7,0xB7,0xB7, 0xC9,0xC9,0xC9, 0xDB,0xDB,0xDB,
	0xED,0xED,0xED, 0xFF,0xFF,0xFF, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00,

	0x00,0x00,0x00, 0x00,0x00,0x00, 0x70,0x70,0x20, 0x88,0x88,0x40, 0xA0,0xA0,0x60, 0xB8,0xB8,0x80, 0xD0,0xD0,0x90, 0xE8,0xE8,0xA0,
	0xF7,0xF7,0xC0, 0xFF,0xFF,0xE0, 0x00,0x00,0x60, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,0x00,0x00
};

//---------------------------------------------------------------------------------
void myVBlank(void) {
//---------------------------------------------------------------------------------
	vblIrqHandler();
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	irqInit();
	irqEnable(IRQ_VBLANK);
	showSplash(getSplashScreen(BWSID));

	setupGraphics();
	irqSet(IRQ_VBLANK, myVBlank);
	setupGUI();
	getInput();
	initSettings();
	bool fsOk = initFileHelper(BWSID);
	loadSettings();
	machineInit();
	loadCart();
	setupEmuBackground();
	loadIntEeproms();
	if (fsOk) {
		loadBioses();
		const RomHeader *rh = findRom(0);
		loadGame(rh);
	}
	else {
		infoOutput("No roms found.");
	}

	while (1) {
		waitVBlank();
//		checkTimeOut();
		guiRunLoop();
		if (!pauseEmulation) {
			if (powerIsOn) {
				run();
			}
			else {
				shutDownLCD();
			}
		}
	}
}

//---------------------------------------------------------------------------------
void pausVBlank(int count) {
//---------------------------------------------------------------------------------
	while (--count) {
		waitVBlank();
	}
}

//---------------------------------------------------------------------------------
void waitVBlank() {
//---------------------------------------------------------------------------------
	IntrWait(0, IRQ_VBLANK);
}

//---------------------------------------------------------------------------------
static void checkTimeOut() {
//---------------------------------------------------------------------------------
	if (EMUinput) {
		sleepTimer = sleepTime;
	}
	else {
		sleepTimer--;
		if (sleepTimer < 0) {
			sleepTimer = sleepTime;
			gbaSleep();
		}
	}
}

//---------------------------------------------------------------------------------
void setEmuSpeed(int speed) {
//---------------------------------------------------------------------------------
	if (speed == 0) {		// Normal Speed
		waitMaskIn = 0x00;
		waitMaskOut = 0x00;
	}
	else if (speed == 1) {	// Double speed
		waitMaskIn = 0x00;
		waitMaskOut = 0x01;
	}
	else if (speed == 2) {	// Max speed (4x)
		waitMaskIn = 0x00;
		waitMaskOut = 0x03;
	}
	else if (speed == 3) {	// 50% speed
		waitMaskIn = 0x01;
		waitMaskOut = 0x00;
	}
}

//---------------------------------------------------------------------------------
static void setupGraphics() {
//---------------------------------------------------------------------------------

	// Set up the display
	GFX_DISPCNT = MODE_0
				| OBJ_1D_MAP
				| BG0_ON
				| BG1_ON
				| BG2_ON
				| BG3_ON
				| OBJ_ON
				| WIN0_ON
				| WIN1_ON
				;
	SetMode(GFX_DISPCNT);
	GFX_BG0CNT = TEXTBG_SIZE_256x512 | BG_MAP_BASE(0) | BG_TILE_BASE(2) | BG_PRIORITY(2);
	GFX_BG1CNT = TEXTBG_SIZE_256x512 | BG_MAP_BASE(2) | BG_TILE_BASE(2) | BG_PRIORITY(1);
	REG_BG0CNT = GFX_BG0CNT;
	REG_BG1CNT = GFX_BG1CNT;
	// Background 2 for border
	REG_BG2CNT = TEXTBG_SIZE_256x256 | BG_MAP_BASE(15) | BG_256_COLOR | BG_TILE_BASE(1) | BG_PRIORITY(1);
	REG_BG2HOFS = 8;	// Center border
	REG_BG2VOFS = 16;

	// Set up background 3 for menu
	REG_BG3CNT = TEXTBG_SIZE_512x256 | BG_MAP_BASE(6) | BG_TILE_BASE(0) | BG_PRIORITY(0);
	menuMap = MAP_BASE_ADR(6);

	LZ77UnCompVram(EmuFontTiles, (void *)VRAM+0x2400);
	setupMenuPalette();
}

void setupMenuPalette() {
	convertPalette(&EMUPALBUFF[0xE0], guiPalette, 32, gGammaValue);
}
