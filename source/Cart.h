#ifndef CART_HEADER
#define CART_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u32 maxRomSize;
extern u32 emuFlags;
extern u8 gMachineSet;
extern u8 gMachine;
extern u8 gSOC;
extern u8 gLang;
extern u8 gPaletteBank;

extern const u8 *romSpacePtr;
extern u8 wsRAM[0x4000];
extern u8 wscRAM[0x10000];
extern u8 SCRATCH_BUFF[0x18000];
extern const void *g_BIOSBASE_BNW;
extern const void *g_BIOSBASE_COLOR;
extern const void *g_BIOSBASE_CRYSTAL;

void machineInit(void);
void loadCart(int emuflags);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !CART_HEADER
