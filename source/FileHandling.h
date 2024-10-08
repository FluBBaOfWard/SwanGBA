#ifndef FILEHANDLING_HEADER
#define FILEHANDLING_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "Emubase.h"
#include "WonderSwan.h"

#define FILEEXTENSIONS ".ws.wsc"

extern ConfigData cfg;

int initSettings(void);
int loadSettings(void);
void saveSettings(void);
bool loadGame(const RomHeader *rh);
void checkMachine(const RomHeader *rh);
void loadNVRAM(void);
void saveNVRAM(void);
void loadState(void);
void saveState(void);
void selectGame(void);
void loadBioses(void);
int loadIntEeproms(void);
void clearIntEeproms(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILEHANDLING_HEADER
