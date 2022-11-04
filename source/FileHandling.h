#ifndef FILEHANDLING_HEADER
#define FILEHANDLING_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "Emubase.h"
#include "WonderSwan.h"

#define FILEEXTENSIONS ".ws.wsc"

extern ConfigData cfg;

int loadSettings(void);
void saveSettings(void);
bool loadGame(const RomHeader *rh);
void checkMachine(const RomHeader *rh);
int loadNVRAM(void);
void saveNVRAM(void);
void loadState(void);
void saveState(void);
void selectGame(void);
void loadBioses(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILEHANDLING_HEADER
