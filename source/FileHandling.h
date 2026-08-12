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
bool loadROM(const u8 *rom, int size, int emuflags);
bool loadGame(const RomHeader *rh);
void checkMachine(int emuflags);
/// Load SRAM, EEPROM and/or Flash if they exist.
int loadNVRAM(void);
/// Save SRAM, EEPROM and/or Flash if they exist.
int saveNVRAM(void);
void loadState(void);
void saveState(void);
void selectGame(void);
void loadBioses(void);
void viewNVRAM(void);
void viewSStates(void);
int loadIntEeproms(void);
int saveIntEeproms(void);
void clearIntEeproms(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !FILEHANDLING_HEADER
