#ifndef EMUBASE
#define EMUBASE

#ifdef __cplusplus
extern "C" {
#endif

#define BWSID 0x1A535742		// "BWS",0x1A - Bandai WonderSwan

#define HALF_CPU_SPEED		(1<<16)
#define ALLOW_SPEED_HACKS	(1<<17)
#define ENABLE_HEADPHONES	(1<<18)

typedef struct {
	const u32 identifier;		// BWSID
	const u32 filesize;
	const u32 flags;			// Bit 1 = PCV2, Bit 2 = WSC, Bit 3 = SwanCrystal.
	const u32 undefined;
	const u8 bios;				// Bit 0 = Bios,
	const u8 reserved[15];
	const char name[32];
	const u8 romData[];
} RomHeader;

typedef struct {				// (config struct)
	int emuSettings;			// Misc standard settings
	int unused;					// unused
	u8 gammaValue;				// from gfx.s
	u8 config;					// from cart.s
	u8 controller;				// from io.s
	u8 name[16];				// Name on start screen
	u8 birthYear[2];			// BCD encoded BIG endian
	u8 birthMonth;				// BCD encoded
	u8 birthDay;				// BCD encoded
	u8 sex;						// 1 = male, 2 = female
	u8 bloodType;				// 1 = A, 2 = B, 3 = O, 4 = AB
	u8 machine;
	u8 palette;
} ConfigData;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !EMUBASE
