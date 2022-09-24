#ifndef FILEHANDLING_HEADER
#define FILEHANDLING_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "Emubase.h"
#include "WonderSwan.h"
#include "RomHeader.h"

#define FILEEXTENSIONS ".ngp.ngc"

extern ConfigData cfg;

int loadSettings(void);
void saveSettings(void);
int loadNVRAM(void);
void saveNVRAM(void);
void loadState(void);
void saveState(void);
void selectGame(void);
bool loadGame(const romheader *rh);
void checkMachine(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILEHANDLING_HEADER
