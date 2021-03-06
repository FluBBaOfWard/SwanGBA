#ifdef __arm__

#include "Equates.h"
#include "WSEEPROM/WSEEPROM.i"
#include "WSRTC/WSRTC.i"
#include "Sphinx/Sphinx.i"
#include "ARMV30MZ/ARMV30MZ.i"

	.global machineInit
	.global loadCart
	.global romNum
	.global cartFlags
	.global romStart
	.global reBankSwitch4_F
	.global reBankSwitch1
	.global reBankSwitch2
	.global reBankSwitch3
	.global BankSwitch4_F_W
	.global BankSwitch1_W
	.global BankSwitch1_L_W
	.global BankSwitch1_H_W
	.global BankSwitch2_W
	.global BankSwitch2_L_W
	.global BankSwitch2_H_W
	.global BankSwitch3_W
	.global BankSwitch3_L_W
	.global BankSwitch3_H_W
	.global clearDirtyTiles

	.global wsHeader
	.global romSpacePtr
	.global MEMMAPTBL_

	.global biosBase
	.global biosSpace
	.global biosSpaceColor
	.global biosSpaceCrystal
	.global g_BIOSBASE_BNW
	.global g_BIOSBASE_COLOR
	.global g_BIOSBASE_CRYSTAL
	.global wsRAM
	.global wsSRAM
	.global extEepromMem
	.global sramSize
	.global eepromSize
	.global gRomSize
	.global maxRomSize
	.global romMask
	.global gGameID
	.global cartOrientation
	.global gConfig
	.global gMachine
	.global gMachineSet
	.global gSOC
	.global gLang
	.global gPaletteBank

	.global extEepromDataLowR
	.global extEepromDataHighR
	.global extEepromAdrLowR
	.global extEepromAdrHighR
	.global extEepromStatusR
	.global extEepromDataLowW
	.global extEepromDataHighW
	.global extEepromAdrLowW
	.global extEepromAdrHighW
	.global extEepromCommandW

	.global cartRtcStatusR
	.global cartRtcCommandW
	.global cartRtcDataR
	.global cartRtcDataW
	.global cartRtcUpdate

	.syntax unified
	.arm

	.section .rodata
	.align 2

ROM_Space:
//	.incbin "wsroms/Anchorz Field (Japan).ws"
//	.incbin "wsroms/Beat Mania (J) [M][!].ws"
//	.incbin "wsroms/Crazy Climber (J) [M][!].ws"
//	.incbin "wsroms/Chaos Demo V2.1 by Charles Doty (PD).wsc"
//	.incbin "wsroms/Guilty Gear Petit (J).wsc"
//	.incbin "wsroms/GunPey (Japan).ws"
	.incbin "wsroms/Kaze no Klonoa - Moonlight Museum (Japan).ws"
//	.incbin "wsroms/Magical Drop for WonderSwan (Japan).ws"
//	.incbin "wsroms/Makaimura for WonderSwan (Japan).ws"
//	.incbin "wsroms/Mr. Driller (J) [!].wsc"
//	.incbin "wsroms/SD Gundam - Operation U.C. (Japan).wsc"
//	.incbin "wsroms/Tetris (Japan).wsc"
//	.incbin "wsroms/Tonpuusou (Japan).wsc"
//	.incbin "wsroms/WONDERPR.WSC"
//	.incbin "wsroms/XI Little (Japan).wsc"
ROM_SpaceEnd:
WS_BIOS_INTERNAL:
	.incbin "wsroms/boot.rom"
//	.incbin "wsroms/ws_irom.bin"
WSC_BIOS_INTERNAL:
SC_BIOS_INTERNAL:
	.incbin "wsroms/boot1.rom"
//	.incbin "wsroms/wc_irom.bin"
//	.incbin "wsroms/wsc_irom.bin"

	.section .ewram,"ax"
	.align 2
;@----------------------------------------------------------------------------
machineInit: 	;@ Called from C
	.type   machineInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}

	ldr r0,=romSize
	mov r1,#ROM_SpaceEnd-ROM_Space
	str r1,[r0]
	ldr r0,=romSpacePtr
	ldr r7,=ROM_Space
	str r7,[r0]

	bl gfxInit
//	bl ioInit
	bl soundInit
	bl cpuInit

	ldmfd sp!,{r4-r11,lr}
	bx lr

	.section .ewram,"ax"
	.align 2
;@----------------------------------------------------------------------------
loadCart: 		;@ Called from C:
	.type   loadCart STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
	ldr v30ptr,=V30OpTable

	ldr r0,romSize
	movs r2,r0,lsr#16		;@ 64kB blocks.
	subne r2,r2,#1
	str r2,romMask			;@ romMask=romBlocks-1

	mov r1,#0xFF
	bl BankSwitch4_F_W
	mov r1,#0xFF
	bl BankSwitch3_W
	mov r1,#0xFF
	bl BankSwitch2_W
	mov r1,#0
	bl BankSwitch1_W

	ldr r1,=wsRAM
	str r1,[v30ptr,#v30MemTblInv-0x1*4]		;@ 0 RAM
	ldr r6,[v30ptr,#v30MemTblInv-0x10*4]	;@ MemMap

	ldr r4,=0xFFFF8			;@ Game ID
	ldrb r0,[r6,r4]
	strb r0,gGameID
	add r4,r4,#3			;@ NVRAM size
	ldrb r3,[r6,r4]
	mov r0,#0				;@ r0 = sram size
	mov r1,#0				;@ r1 = eeprom size
	cmp r3,#0x01			;@ 64kbit sram
	moveq r0,#0x2000
	cmp r3,#0x02			;@ 256kbit sram
	moveq r0,#0x8000
	cmp r3,#0x03			;@ 1Mbit sram
	moveq r0,#0x20000
	cmp r3,#0x04			;@ 2Mbit sram
	moveq r0,#0x40000
	cmp r3,#0x05			;@ 4Mbit sram
	moveq r0,#0x80000
	cmp r3,#0x10			;@ 1kbit eeprom
	moveq r1,#0x80
	cmp r3,#0x20			;@ 16kbit eeprom
	moveq r1,#0x800
	cmp r3,#0x50			;@ 8kbit eeprom
	moveq r1,#0x400
	str r0,sramSize
	str r1,eepromSize

	add r4,r4,#1			;@ Orientation
	ldrb r0,[r6,r4]
	and r0,r0,#1
	strb r0,cartOrientation

	add r4,r4,#1			;@ RTC present
	ldrb r0,[r6,r4]
	strb r0,rtcPresent

	ldrb r5,gMachine
	cmp r5,#HW_WONDERSWAN
	cmpne r5,#HW_POCKETCHALLENGEV2
	moveq r0,#1				;@ Set boot rom overlay (size small)
	ldreq r1,g_BIOSBASE_BNW
	ldreq r2,=WS_BIOS_INTERNAL
	moveq r4,#SOC_ASWAN
	movne r0,#2				;@ Set boot rom overlay (size big)
	ldrne r1,g_BIOSBASE_COLOR
	ldrne r2,=WSC_BIOS_INTERNAL
	movne r4,#SOC_SPHINX
	cmp r5,#HW_SWANCRYSTAL
	ldreq r1,g_BIOSBASE_CRYSTAL
	ldreq r2,=SC_BIOS_INTERNAL
	moveq r4,#SOC_SPHINX2
	strb r4,gSOC
	cmp r1,#0
	moveq r1,r2				;@ Use internal bios
	str r1,biosBase
	bl setBootRomOverlay


	ldr r0,=wsRAM			;@ Clear RAM
	mov r1,#0x10000/4
	bl memclr_
	bl clearDirtyTiles
	cmp r4,#SOC_ASWAN
	ldreq r0,=wsRAM+0x4000	;@ Clear mem outside of RAM
	moveq r1,#0x90
	moveq r2,#0xC000
	bleq memset

//	bl hacksInit
	bl extEepromReset
	bl cartRtcReset
	bl gfxReset
	bl ioReset
	bl soundReset
	bl cpuReset
	ldmfd sp!,{r4-r11,lr}
	bx lr


;@----------------------------------------------------------------------------
clearDirtyTiles:
;@----------------------------------------------------------------------------
	ldr r0,=DIRTYTILES			;@ Clear RAM
	mov r1,#0x800/4
	b memclr_
;@----------------------------------------------------------------------------
reBankSwitch4_F:					;@ 0x40000-0xFFFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	ldrb r1,[spxptr,#wsvBnk0SlctX]
;@----------------------------------------------------------------------------
BankSwitch4_F_W:					;@ 0x40000-0xFFFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	strb r1,[spxptr,#wsvBnk0SlctX]
	mov r1,r1,lsl#4
	orr r1,r1,#4

	ldr r0,romMask
	ldr r2,romSpacePtr
	sub r2,r2,#0x40000
	add r12,v30ptr,#v30MemTblInv-4*4
tbLoop2:
	and r3,r0,r1
	add r3,r2,r3,lsl#16		;@ 64kB blocks.
	sub r2,r2,#0x10000
	str r3,[r12,#-4]!
	add r1,r1,#1
	ands r3,r1,#0xF
	bne tbLoop2

	bx lr
;@----------------------------------------------------------------------------
BankSwitch1_H_W:				;@ 0x10000-0x1FFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	strb r1,[spxptr,#wsvBnk1SlctX+1]
;@----------------------------------------------------------------------------
reBankSwitch1:					;@ 0x10000-0x1FFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	ldrh r1,[spxptr,#wsvBnk1SlctX]
;@----------------------------------------------------------------------------
BankSwitch1_W:					;@ 0x10000-0x1FFFF
BankSwitch1_L_W:				;@ 0x10000-0x1FFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	strb r1,[spxptr,#wsvBnk1SlctX]

	ldr r0,sramSize
	movs r0,r0,lsr#16		;@ 64kB blocks.
	subne r0,r0,#1
	ldr r2,=wsSRAM-0x10000
	and r3,r1,r0
	add r3,r2,r3,lsl#16		;@ 64kB blocks.
	str r3,[v30ptr,#v30MemTblInv-2*4]

	bx lr
;@----------------------------------------------------------------------------
BankSwitch2_H_W:				;@ 0x20000-0x2FFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	strb r1,[spxptr,#wsvBnk2SlctX+1]
;@----------------------------------------------------------------------------
reBankSwitch2:					;@ 0x20000-0x2FFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	ldrb r1,[spxptr,#wsvBnk2SlctX]
;@----------------------------------------------------------------------------
BankSwitch2_W:					;@ 0x20000-0x2FFFF
BankSwitch2_L_W:				;@ 0x20000-0x2FFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	strb r1,[spxptr,#wsvBnk2SlctX]

	ldr r0,romMask
	ldr r2,romSpacePtr
	sub r2,r2,#0x20000
	and r3,r1,r0
	add r3,r2,r3,lsl#16		;@ 64kB blocks.
	str r3,[v30ptr,#v30MemTblInv-3*4]

	bx lr

;@----------------------------------------------------------------------------
BankSwitch3_H_W:				;@ 0x30000-0x3FFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	strb r1,[spxptr,#wsvBnk3SlctX+1]
;@----------------------------------------------------------------------------
reBankSwitch3:					;@ 0x30000-0x3FFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	ldrh r1,[spxptr,#wsvBnk3SlctX]
;@----------------------------------------------------------------------------
BankSwitch3_W:					;@ 0x30000-0x3FFFF
BankSwitch3_L_W:				;@ 0x30000-0x3FFFF
;@----------------------------------------------------------------------------
	ldr spxptr,=sphinx0
	strb r1,[spxptr,#wsvBnk3SlctX]

	ldr r0,romMask
	ldr r2,romSpacePtr
	sub r2,r2,#0x30000
	and r3,r1,r0
	add r3,r2,r3,lsl#16		;@ 64kB blocks.
	str r3,[v30ptr,#v30MemTblInv-4*4]

	bx lr

;@----------------------------------------------------------------------------
extEepromDataLowR:
;@----------------------------------------------------------------------------
	adr eeptr,extEeprom
	b wsEepromDataLowR
;@----------------------------------------------------------------------------
extEepromDataHighR:
;@----------------------------------------------------------------------------
	adr eeptr,extEeprom
	b wsEepromDataHighR
;@----------------------------------------------------------------------------
extEepromAdrLowR:
;@----------------------------------------------------------------------------
	adr eeptr,extEeprom
	b wsEepromAddressLowR
;@----------------------------------------------------------------------------
extEepromAdrHighR:
;@----------------------------------------------------------------------------
	adr eeptr,extEeprom
	b wsEepromAddressHighR
;@----------------------------------------------------------------------------
extEepromStatusR:
;@----------------------------------------------------------------------------
	adr eeptr,extEeprom
	b wsEepromStatusR
;@----------------------------------------------------------------------------
extEepromDataLowW:
;@----------------------------------------------------------------------------
	adr eeptr,extEeprom
	b wsEepromDataLowW
;@----------------------------------------------------------------------------
extEepromDataHighW:
;@----------------------------------------------------------------------------
	adr eeptr,extEeprom
	b wsEepromDataHighW
;@----------------------------------------------------------------------------
extEepromAdrLowW:
;@----------------------------------------------------------------------------
	adr eeptr,extEeprom
	b wsEepromAddressLowW
;@----------------------------------------------------------------------------
extEepromAdrHighW:
;@----------------------------------------------------------------------------
	adr eeptr,extEeprom
	b wsEepromAddressHighW
;@----------------------------------------------------------------------------
extEepromCommandW:
;@----------------------------------------------------------------------------
	adr eeptr,extEeprom
	b wsEepromCommandW
;@----------------------------------------------------------------------------
extEepromReset:
;@----------------------------------------------------------------------------
	ldr r1,eepromSize
	cmp r1,#0
	bxeq lr
	ldr r2,=extEepromMem
	adr eeptr,extEeprom
	b wsEepromReset
;@----------------------------------------------------------------------------
extEeprom:
	.space wsEepromSize

;@----------------------------------------------------------------------------
cartRtcStatusR:
;@----------------------------------------------------------------------------
	adr rtcptr,cartRtc
	b wsRtcStatusR
;@----------------------------------------------------------------------------
cartRtcCommandW:
;@----------------------------------------------------------------------------
	adr rtcptr,cartRtc
	b wsRtcCommandW
;@----------------------------------------------------------------------------
cartRtcDataR:
;@----------------------------------------------------------------------------
	adr rtcptr,cartRtc
	b wsRtcDataR
;@----------------------------------------------------------------------------
cartRtcDataW:
;@----------------------------------------------------------------------------
	adr rtcptr,cartRtc
	b wsRtcDataW
;@----------------------------------------------------------------------------
cartRtcReset:
;@----------------------------------------------------------------------------
	ldrb r0,rtcPresent
	cmp r0,#0
	bxeq lr
	stmfd sp!,{lr}
	adr rtcptr,cartRtc
	ldr r1,=setInterruptExternal
	bl wsRtcReset
	bl getTime					;@ r0 = ??ssMMHH, r1 = ??DDMMYY
	ldmfd sp!,{lr}
	mov r2,r1
	mov r1,r0
	adr rtcptr,cartRtc
	b wsRtcSetDateTime
;@----------------------------------------------------------------------------
cartRtcUpdate:		;@ r0=rtcptr. Call every second.
;@----------------------------------------------------------------------------
	ldrb r0,rtcPresent
	cmp r0,#0
	bxeq lr
	adr rtcptr,cartRtc
	b wsRtcUpdate
;@----------------------------------------------------------------------------
cartRtc:
	.space wsRtcSize

romNum:
	.long 0						;@ romnumber
romInfo:						;@
emuFlags:
	.byte 0						;@ emuflags      (label this so UI.C can take a peek) see equates.h for bitfields
//scaling:
	.byte 0						;@ (display type)
	.byte 0,0					;@ (sprite follow val)
cartFlags:
	.byte 0 					;@ cartflags
gConfig:
	.byte 0						;@ Config, bit 7=BIOS on/off
gMachineSet:
	.byte HW_AUTO
gMachine:
	.byte HW_WONDERSWANCOLOR
gSOC:
	.byte SOC_SPHINX
gLang:
	.byte 1						;@ language
gPaletteBank:
	.byte 0						;@ palettebank
gGameID:
	.byte 0						;@ Game ID
rtcPresent:
	.byte 0						;@ RTC present in cartridge
cartOrientation:
	.byte 0						;@ 1=Vertical, 0=Horizontal
	.space 2					;@ alignment.

wsHeader:
romSpacePtr:
	.long 0
g_BIOSBASE_BNW:
	.long 0
g_BIOSBASE_COLOR:
	.long 0
g_BIOSBASE_CRYSTAL:
	.long 0
gRomSize:
romSize:
	.long 0
maxRomSize:
	.long 0
romMask:
	.long 0
biosBase:
	.long 0
sramSize:
	.long 0
eepromSize:
	.long 0

#ifdef GBA
	.section .sbss				;@ For the GBA
#else
	.section .bss
#endif
	.align 2
wsRAM:
	.space 0x10000
wsSRAM:
#ifdef GBA
	.space 0x10000				;@ For the GBA
#else
	.space 0x40000
#endif

biosSpace:
	.space 0x1000
biosSpaceColor:
	.space 0x2000
biosSpaceCrystal:
	.space 0x2000
extEepromMem:
	.space 0x800
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
