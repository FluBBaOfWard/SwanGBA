#ifdef __arm__

//#define EMBEDDED_ROM

#include "Sphinx/Sphinx.i"
#include "ARMV30MZ/ARMV30MZ.i"

	.global biosBase
	.global biosSpace
	.global biosSpaceColor
	.global biosSpaceCrystal
	.global g_BIOSBASE_BNW
	.global g_BIOSBASE_COLOR
	.global g_BIOSBASE_CRYSTAL
	.global wsRAM
	.global DIRTYTILES
	.global maxRomSize
	.global gMachineSet
	.global gMachine
	.global gSOC
	.global gLang
	.global gPaletteBank

	.global machineInit
	.global loadCart
	.global clearDirtyTiles

	.syntax unified
	.arm

	.section .rodata
	.align 2

#ifdef EMBEDDED_ROM
ROM_Space:
//	.incbin "wsroms/Anchorz Field (Japan).ws"
//	.incbin "wsroms/Beat Mania (J) [M][!].ws"
//	.incbin "wsroms/Blue Wing Blitz (Japan).wsc"
//	.incbin "wsroms/Crazy Climber (J) [M][!].ws"
//	.incbin "wsroms/Chaos Demo V2.1 by Charles Doty (PD).wsc"
//	.incbin "wsroms/Chaos Gear - Michibikareshi Mono (Japan).ws"
//	.incbin "wsroms/Dicing Knight. (J).wsc"
//	.incbin "wsroms/dkd-vesilintu.wsc"
//	.incbin "wsroms/Final Fantasy (Japan).wsc"
//	.incbin "wsroms/Final Fantasy IV (Japan).wsc"
//	.incbin "wsroms/Final Lap 2000 (Japan).ws"
//	.incbin "wsroms/Finally.emu.wsc"
//	.incbin "wsroms/Finally.hw.wsc"
//	.incbin "wsroms/Front Mission (Japan).wsc"
//	.incbin "wsroms/Guilty Gear Petit (J).wsc"
//	.incbin "wsroms/GunPey (Japan).ws"
//	.incbin "wsroms/Hanjuku Hero - Ah, Sekai yo Hanjuku Nare...!! (Japan).wsc"
//	.incbin "wsroms/Hataraku Chocobo (Japan).wsc"
//	.incbin "wsroms/Kaze no Klonoa - Moonlight Museum (Japan).ws"
//	.incbin "wsroms/Macross - True Love Song (Japan).ws"
//	.incbin "wsroms/Magical Drop for WonderSwan (Japan).ws"
//	.incbin "wsroms/Mahjong Touryuumon (Japan).ws"
//	.incbin "wsroms/Makai Toushi Sa-Ga (Japan).wsc"
//	.incbin "wsroms/Makaimura for WonderSwan (Japan).ws"
//	.incbin "wsroms/Mingle Magnet (Japan).ws"
//	.incbin "wsroms/Mr. Driller (J) [!].wsc"
//	.incbin "wsroms/Nazo Ou Pocket (Japan).ws"
//	.incbin "wsroms/Romancing Sa-Ga (Japan).wsc"
//	.incbin "wsroms/SD Gundam - Operation U.C. (Japan).wsc"
//	.incbin "wsroms/Shinkenzemi Chuugaku Kouza - Chuu1 Suugaku (Japan).pc2"
//	.incbin "wsroms/Side Pocket for WonderSwan (Japan).ws"
//	.incbin "wsroms/Tetris (Japan).wsc"
//	.incbin "wsroms/Time Bokan Series - Bokan Densetsu - Butamo Odaterya Doronbou (Japan).ws"
//	.incbin "wsroms/timingtest.ws"
//	.incbin "wsroms/Tonpuusou (Japan).wsc"
//	.incbin "wsroms/Ultraman - Hikari no Kuni no Shisha (Japan).wsc"
//	.incbin "wsroms/With You - Mitsumete Itai (Japan).wsc"
//	.incbin "wsroms/WONDERPR.WSC"
//	.incbin "wsroms/WSHWTest.wsc"
//	.incbin "wsroms/XI Little (Japan).wsc"
ROM_SpaceEnd:
WS_BIOS_INTERNAL:
	.incbin "wsroms/boot.rom"
WSC_BIOS_INTERNAL:
	.incbin "wsroms/boot1.rom"
SC_BIOS_INTERNAL:
	.incbin "wsroms/boot2.rom"
#else
WS_BIOS_INTERNAL:
	.incbin "wsroms/ws_irom.bin"
WSC_BIOS_INTERNAL:
SC_BIOS_INTERNAL:
	.incbin "wsroms/wc_irom.bin"
#endif

	.section .ewram,"ax"
	.align 2
;@----------------------------------------------------------------------------
machineInit: 				;@ Called from C
	.type   machineInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}

#ifdef EMBEDDED_ROM
	ldr r0,=romSize
	mov r1,#ROM_SpaceEnd-ROM_Space
	str r1,[r0]
	ldr r0,=romSpacePtr
	ldr r7,=ROM_Space
	str r7,[r0]
#endif

	bl gfxInit
//	bl ioInit
	bl soundInit
	bl cpuInit

	ldmfd sp!,{r4-r11,lr}
	bx lr

	.section .ewram,"ax"
	.align 2
;@----------------------------------------------------------------------------
loadCart: 					;@ Called from C:
	.type   loadCart STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}

	bl wsCartReset

	ldr v30ptr,=V30OpTable
	ldr r2,[v30ptr,#v30MemTblInv-0x10*4]	;@ MemMap

	ldr r4,=0xFFFF0				;@ Header offset
	add r4,r2,r4
	ldrb r2,[r4,#0x7]			;@ Color

	ldrb r5,gMachineSet
	cmp r5,#HW_AUTO
	bne noHWCheck
	tst r2,#1					;@ Color from header
	moveq r5,#HW_WONDERSWAN
	movne r5,#HW_WONDERSWANCOLOR
noHWCheck:
	strb r5,gMachine
	cmp r5,#HW_WONDERSWAN
	cmpne r5,#HW_POCKETCHALLENGEV2
	moveq r0,#1					;@ Set boot rom overlay (size small)
	ldreq r1,g_BIOSBASE_BNW
	ldreq r2,=WS_BIOS_INTERNAL
	moveq r4,#SOC_ASWAN
	movne r0,#2					;@ Set boot rom overlay (size big)
	ldrne r1,g_BIOSBASE_COLOR
	ldrne r2,=WSC_BIOS_INTERNAL
	movne r4,#SOC_SPHINX
	cmp r5,#HW_SWANCRYSTAL
	ldreq r1,g_BIOSBASE_CRYSTAL
	ldreq r2,=SC_BIOS_INTERNAL
	moveq r4,#SOC_SPHINX2
	strb r4,gSOC
	cmp r1,#0
	moveq r1,r2					;@ Use internal bios
	str r1,biosBase
	bl setBootRomOverlay


	ldr r0,=wsRAM				;@ Clear RAM
	str r0,[v30ptr,#v30MemTblInv-0x1*4]		;@ 0 RAM
	mov r1,#0x10000/4
	bl memclr_
	bl clearDirtyTiles
	cmp r4,#SOC_ASWAN
	ldreq r0,=wsRAM+0x4000		;@ Clear mem outside of RAM
	moveq r1,#0x90
	moveq r2,#0xC000
	bleq memset

	bl hacksInit
	bl gfxReset
	bl resetCartridgeBanks
	bl ioReset
	bl soundReset
	mov r0,r4					;@ SOC
	bl cpuReset
	ldmfd sp!,{r4-r11,lr}
	bx lr


;@----------------------------------------------------------------------------
clearDirtyTiles:
;@----------------------------------------------------------------------------
	ldr r0,=DIRTYTILES			;@ Clear RAM
	mov r1,#0x800/4
	b memclr_

romInfo:						;@
emuFlags:
	.byte 0						;@ emuflags      (label this so GUI.C can take a peek) see EmuSettings.h for bitfields
	.byte 0						;@ (display type)
	.byte 0,0					;@ (sprite follow val)
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
	.space 3					;@ alignment.

allocatedRomMem:
	.long 0
g_BIOSBASE_BNW:
	.long 0
g_BIOSBASE_COLOR:
	.long 0
g_BIOSBASE_CRYSTAL:
	.long 0
maxRomSize:
	.long 0
biosBase:
	.long 0

;@----------------------------------------------------------------------------
#ifdef GBA
	.section .sbss				;@ For the GBA
#else
	.section .bss
#endif
	.align 4
wsRAM:
	.space 0x10000
DIRTYTILES:
	.space 0x800
biosSpace:
	.space 0x1000
biosSpaceColor:
	.space 0x2000
biosSpaceCrystal:
	.space 0x2000
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
