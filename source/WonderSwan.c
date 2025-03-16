#include <gba.h>
#include <string.h>

#include "WonderSwan.h"
#include "WSBorder.h"
#include "WSCBorder.h"
#include "PCV2Border.h"
#include "Gui.h"
#include "Cart.h"
#include "Gfx.h"
#include "ARMV30MZ/ARMV30MZ.h"


int packState(void *statePtr) {
	int size = 0;
	memcpy(statePtr+size, wsRAM, sizeof(wsRAM));
	size += sizeof(wsRAM);
	size += sphinxSaveState(statePtr+size, &sphinx0);
	size += V30SaveState(statePtr+size, &V30OpTable);
	memcpy(statePtr+size, wsSRAM, sizeof(wsSRAM));
	size += sizeof(wsSRAM);
	size += wsEepromSaveState(statePtr+size, &extEeprom);
	return size;
}

void unpackState(const void *statePtr) {
	int size = 0;
	memcpy(wsRAM, statePtr+size, sizeof(wsRAM));
	size += sizeof(wsRAM);
	size += sphinxLoadState(&sphinx0, statePtr+size);
	size += V30LoadState(&V30OpTable, statePtr+size);
	memcpy(wsSRAM, statePtr+size, sizeof(wsSRAM));
	size += sizeof(wsSRAM);
	size += wsEepromLoadState(&extEeprom, statePtr+size);
}

int getStateSize() {
	int size = 0;
	size += sizeof(wsRAM);
	size += sphinxGetStateSize();
	size += V30GetStateSize();
	size += sizeof(wsSRAM);
	size += wsEepromGetStateSize();
	return size;
}

static void setupBorderPalette(const unsigned short *palette, int len) {
	if (gBorderEnable == 0) {
		memset(EMUPALBUFF, 0, len);
	}
	else {
		memcpy(EMUPALBUFF, palette, len);
	}
	paletteTxAll();					// Make new palette visible
}

static void setupBorderTiles(const void *tiles) {
	LZ77UnCompVram(tiles, TILE_BASE_ADR(1));
}

static void setupBorderMap(const void *map) {
	LZ77UnCompVram(map, MAP_BASE_ADR(15));
}

void setupWSBorderPalette() {
	setupBorderPalette(WSBorderPal, WSBorderPalLen);
}

void setupWSCBorderPalette() {
	setupBorderPalette(WSCBorderPal, WSCBorderPalLen);
}

void setupPCV2BorderPalette() {
	setupBorderPalette(PCV2BorderPal, PCV2BorderPalLen);
}

static void fillScreenMap(u16 val) {
	u16 *dest = MAP_BASE_ADR(15);
	u16 fill = dest[val];
	for (int i=3;i<21;i++) {
		for (int j=2;j<30;j++) {
			dest[i*32+j] = fill;
		}
	}
}

static void fillScreenRow(u16 val, int row) {
	u16 *dest = MAP_BASE_ADR(15);
	u16 fill = dest[val];
	dest += (3 * 32);
	for (int j=2;j<30;j++) {
		dest[row*32+j] = fill;
	}
}

void setupEmuBackground() {
	if (gMachine == HW_WONDERSWANCOLOR || gMachine == HW_SWANCRYSTAL) {
		setupBorderTiles(WSCBorderTiles);
		setupBorderMap(WSCBorderMap);
		setupWSCBorderPalette();
	}
	else if (gMachine == HW_WONDERSWAN) {
		setupBorderTiles(WSBorderTiles);
		setupBorderMap(WSBorderMap);
		setupWSBorderPalette();
	}
	else {
		setupBorderTiles(PCV2BorderTiles);
		setupBorderMap(PCV2BorderMap);
		setupPCV2BorderPalette();
	}
}

void setupEmuBgrShutDown() {
	if (gMachine == HW_WONDERSWANCOLOR) {
		setupBorderMap(WSCBorderMap);
		fillScreenMap(0x31E);
		fillScreenRow(0x31F, 5);
	}
	else if (gMachine == HW_SWANCRYSTAL) {
		setupBorderMap(WSCBorderMap);
		fillScreenMap(0x31F);
	}
	else if (gMachine == HW_WONDERSWAN) {
		setupBorderMap(WSBorderMap);
		fillScreenMap(0x31F);
	}
	else {
		setupBorderMap(PCV2BorderMap);
		fillScreenMap(0x31F);
	}
}

void setupEmuBorderPalette() {
	if (gMachine == HW_WONDERSWANCOLOR || gMachine == HW_SWANCRYSTAL) {
		setupWSCBorderPalette();
	}
	else if (gMachine == HW_WONDERSWAN) {
		setupWSBorderPalette();
	}
	else {
		setupPCV2BorderPalette();
	}
}
