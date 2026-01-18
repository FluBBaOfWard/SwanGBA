#ifndef FILEHANDLING_HEADER
#define FILEHANDLING_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "Emubase.h"
#include "WonderSwan.h"

#define FILEEXTENSIONS ".ws.wsc"

extern ConfigData cfg;

void initSettings(void);
bool updateSettingsFromWS(void);
/// Load user settings and internal eeprom.
int loadSettings(void);
/// Save user settings and internal eeprom.
void saveSettings(void);
bool loadGame(const RomHeader *rh);
void checkMachine(const RomHeader *rh);
/// Load SRAM, EEPROM and/or Flash if they exist.
void loadNVRAM(void);
/// Save SRAM, EEPROM and/or Flash if they exist.
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
