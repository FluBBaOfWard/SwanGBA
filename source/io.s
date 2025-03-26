#ifdef __arm__

#include "ARMV30MZ/ARMV30MZ.i"
#include "Sphinx/Sphinx.i"
#include "WSEEPROM/WSEEPROM.i"

	.global joyCfg
	.global EMUinput
	.global batteryLevel
	.global wsEepromMem
	.global wscEepromMem
	.global scEepromMem
	.global intEeprom

	.global ioReset
	.global refreshEMUjoypads
	.global ioSaveState
	.global ioLoadState
	.global ioGetStateSize
	.global updateSlowIO
	.global IOPortA_R

	.global intEepromSetSize
	.global intEepromDataLowR
	.global intEepromDataHighR
	.global intEepromAdrLowR
	.global intEepromAdrHighR
	.global intEepromStatusR
	.global intEepromDataLowW
	.global intEepromDataHighW
	.global intEepromAdrLowW
	.global intEepromAdrHighW
	.global intEepromCommandW

	.syntax unified
	.arm

#ifdef GBA
	.section .ewram, "ax", %progbits	;@ For the GBA
#else
	.section .text						;@ For anything else
#endif
	.align 2
;@----------------------------------------------------------------------------
ioReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	bl intEepromReset

	ldmfd sp!,{pc}
;@----------------------------------------------------------------------------
ioSaveState:				;@ In r0=destination. Out r0=size.
	.type   ioSaveState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

//	ldr r1,=rtcRegs
//	mov r2,#0x100
//	bl memcpy

	ldmfd sp!,{lr}
	mov r0,#0x100
	bx lr
;@----------------------------------------------------------------------------
ioLoadState:				;@ In r0=source. Out r0=size.
	.type   ioLoadState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

//	bl initSysMem

	ldmfd sp!,{lr}
;@----------------------------------------------------------------------------
ioGetStateSize:				;@ Out r0=state size.
	.type   ioGetStateSize STT_FUNC
;@----------------------------------------------------------------------------
	mov r0,#0x100
	bx lr
;@----------------------------------------------------------------------------
refreshEMUjoypads:			;@ Call every frame
;@----------------------------------------------------------------------------

	ldr r4,EMUinput
	and r0,r4,#0xf0
	ldr r1,=gMachine
	ldrb r1,[r1]
	cmp r1,#HW_POCKETCHALLENGEV2
	beq pcv2Joypad
	mov r3,r4
		ldr r1,=frameTotal
		ldr r1,[r1]
		movs r1,r1,lsr#2		;@ C=frame&2 (autofire alternates every 4:th frame)
		ldr r2,joyCfg
		andcs r3,r3,r2
		tstcs r3,r3,lsr#10		;@ NDS L?
		andcs r3,r3,r2,lsr#16
	adr r1,rlud2urdl
	ldrb r0,[r1,r0,lsr#4]

	ldr spxptr,=sphinx0
	ldrb r1,[spxptr,#wsvOrientation]
	cmp r1,#0
	bne verticalJoypad

	tst r4,#0x200				;@ NDS L?
	movne r0,r0,lsl#4			;@ Map dpad to X or Y keys.

	tst r4,#0x08				;@ NDS Start
	orrne r0,r0,#WS_KEY_START	;@ WS Start
	tst r4,#0x04				;@ NDS Select
	orrne r0,r0,#WS_KEY_SOUND	;@ WS Sound

	ands r1,r3,#3				;@ A/B buttons
	cmpne r1,#3
	eorne r1,r1,#3
	tst r2,#0x400				;@ Swap A/B?
	andeq r1,r3,#3
	orr r0,r0,r1,lsl#2

	ldr spxptr,=sphinx0
	b wsvSetJoyState
;@----------------------------------------------------------------------------
verticalJoypad:
;@----------------------------------------------------------------------------
	mov r0,r0,lsl#4				;@ Map dpad to Y keys.

	tst r4,#0x08				;@ NDS Start
	orrne r0,r0,#WS_KEY_START	;@ WS Start
	tst r4,#0x04				;@ NDS Select
	orrne r0,r0,#WS_KEY_SOUND	;@ WS Sound

	and r1,r4,#0x3				;@ A/B buttons
	and r2,r4,#0xC00			;@ X/Y buttons
	orr r1,r1,r2,lsr#8
	adr r2,abxy2xpad
	ldrb r1,[r2,r1]
	orr r0,r0,r1,lsl#4

	ldr spxptr,=sphinx0
	b wsvSetJoyState
;@----------------------------------------------------------------------------
pcv2Joypad:
;@----------------------------------------------------------------------------
	adr r1,rlud2ldur
	ldrb r0,[r1,r0,lsr#4]
	and r3,r4,#0xf00
	adr r1,rlxy2pcv2
	ldrb r1,[r1,r3,lsr#8]
	orr r0,r0,r1,lsl#4

	tst r4,#0x01				;@ NDS A
	orrne r0,r0,#PCV2_KEY_CLEAR	;@ PCV2 Clear
	tst r4,#0x02				;@ NDS B
	orrne r0,r0,#PCV2_KEY_CIRCLE	;@ PCV2 Circle
	ldr r1,=0x2222
	orr r0,r0,r1

	ldr spxptr,=sphinx0
	b wsvSetJoyState
;@----------------------------------------------------------------------------
joyCfg: .long 0x00ff01ff	;@ byte0=auto mask, byte1=(saves R), byte2=R auto mask
							;@ bit 31=single/multi, 30,29=1P/2P, 27=(multi) link active, 24=reset signal received
playerCount:.long 0			;@ Number of players in multilink.
			;@  Bit: 11   10     9  8     7     6      5  4    3  2    1     0
			;@   WS: Y4   Y3     Y2 Y1    X4    X3     X2 X1   B  A    Start 0
			;@ PCV2: Pass Circle 1  Clear Right Escape 1  View Up Down 1     Left
joy0State:	.long 0
rlud2urdl:	.byte 0x00,0x20,0x80,0xA0, 0x10,0x30,0x90,0xB0, 0x40,0x60,0xC0,0xE0, 0x50,0x70,0xD0,0xF0
rlud2ldur:	.byte 0x00,0x80,0x01,0x81, 0x08,0x88,0x09,0x89, 0x04,0x84,0x05,0x85, 0x0C,0x8C,0x0D,0x8D
abxy2xpad:	.byte 0x00,0x02,0x04,0x06, 0x01,0x03,0x05,0x07, 0x08,0x0A,0x0C,0x0E, 0x09,0x0B,0x0D,0x0F
rlxy2pcv2:	.byte 0x00,0x01,0x04,0x05, 0x01,0x01,0x05,0x05, 0x80,0x81,0x84,0x85, 0x81,0x81,0x85,0x85

EMUinput:	.long 0				;@ This label here for Main.c to use

;@----------------------------------------------------------------------------
updateSlowIO:				;@ Call once every frame, updates battery levels and rtc.
;@----------------------------------------------------------------------------
	ldrb r0,slowTimer
	subs r0,r0,#1
	strbhi r0,slowTimer
	bxhi lr
	ldr r0,=fpsTarget
	ldrb r0,[r0]
	strb r0,slowTimer

	stmfd sp!,{r12,lr}
//	blx getBatteryLevel
	ldr r0,batteryLevel
	subs r0,r0,#1
	movmi r0,#1
	str r0,batteryLevel
	cmp r0,#60
	movpl r0,#0
	movmi r0,#1
	bl setLowBattery
	ldmfd sp!,{r12,lr}

	ldr r0,=(3072000/8)			;@ 1 Second in cart clocks (384000).
	b cartUpdate

;@----------------------------------------------------------------------------
batteryLevel:
	.long 0x15000				;@ Around 24h (60*60*24)
slowTimer:
	.byte 0
	.byte 0
	.byte 0
	.byte 0
;@----------------------------------------------------------------------------

;@----------------------------------------------------------------------------
intEepromDataLowR:			;@ 0xBA
;@----------------------------------------------------------------------------
	adr eeptr,intEeprom
	b wsEepromDataLowR
;@----------------------------------------------------------------------------
intEepromDataHighR:			;@ 0xBB
;@----------------------------------------------------------------------------
	adr eeptr,intEeprom
	b wsEepromDataHighR
;@----------------------------------------------------------------------------
intEepromAdrLowR:			;@ 0xBC
;@----------------------------------------------------------------------------
	adr eeptr,intEeprom
	b wsEepromAddressLowR
;@----------------------------------------------------------------------------
intEepromAdrHighR:			;@ 0xBD
;@----------------------------------------------------------------------------
	adr eeptr,intEeprom
	b wsEepromAddressHighR
;@----------------------------------------------------------------------------
intEepromStatusR:			;@ 0xBE
;@----------------------------------------------------------------------------
	adr eeptr,intEeprom
	b wsEepromStatusR
;@----------------------------------------------------------------------------
intEepromDataLowW:			;@ 0xBA, r1 = value
;@----------------------------------------------------------------------------
	mov r1,r0
	adr eeptr,intEeprom
	b wsEepromDataLowW
;@----------------------------------------------------------------------------
intEepromDataHighW:			;@ 0xBB, r1 = value
;@----------------------------------------------------------------------------
	mov r1,r0
	adr eeptr,intEeprom
	b wsEepromDataHighW
;@----------------------------------------------------------------------------
intEepromAdrLowW:			;@ 0xBC, r1 = value
;@----------------------------------------------------------------------------
	mov r1,r0
	adr eeptr,intEeprom
	b wsEepromAddressLowW
;@----------------------------------------------------------------------------
intEepromAdrHighW:			;@ 0xBD, r1 = value
;@----------------------------------------------------------------------------
	mov r1,r0
	adr eeptr,intEeprom
	b wsEepromAddressHighW
;@----------------------------------------------------------------------------
intEepromCommandW:			;@ 0xBE, r1 = value
;@----------------------------------------------------------------------------
	mov r1,r0
	adr eeptr,intEeprom
	b wsEepromCommandW
;@----------------------------------------------------------------------------
intEepromReset:
;@----------------------------------------------------------------------------
	ldr r0,=gSOC
	ldrb r0,[r0]
	cmp r0,#SOC_SPHINX
	ldrmi r2,=wsEepromMem
	ldreq r2,=wscEepromMem
	ldrhi r2,=scEepromMem
	mov r1,#0x080				;@  1kbit, 16kbit is switched to for Color _Games_.
	adr eeptr,intEeprom
	b wsEepromReset
;@----------------------------------------------------------------------------
intEepromSetSize:			;@ r0 = size, 0=1kbit, !0=16kbit
;@----------------------------------------------------------------------------
	cmp r0,#0
	moveq r1,#0x080				;@  1kbit
	movne r1,#0x800				;@ 16kbit
	adr eeptr,intEeprom
	b wsEepromSetSize
;@----------------------------------------------------------------------------
	.pool
intEeprom:
	.space wsEepromSize
wsEepromMem:
	.space 0x80
wscEepromMem:
	.space 0x800
scEepromMem:
	.space 0x800
;@----------------------------------------------------------------------------

	.end
#endif // #ifdef __arm__
