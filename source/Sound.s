#ifdef __arm__

#include "Shared/gba_asm.h"
#include "Sphinx/Sphinx.i"

/// Low res
#define MIX_LEN (304)
/// High res
//#define MIX_LEN (528)
#define SHIFTVAL (21)

	.global soundMode

	.global soundInit
	.global soundReset
	.global soundSwapBuffers
	.global soundRender
	.global soundSetMuteGUI
	.global soundSetMuteChip
	.global mix8Vol

	.extern pauseEmulation

	.syntax unified
	.arm

	.section .ewram, "ax", %progbits
	.align 2
;@----------------------------------------------------------------------------
soundInit:
	.type soundInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r5,lr}
	mov r5,#REG_BASE

;@	ldrh r0,[r5,#REG_SGBIAS]
;@	bic r0,r0,#0xc000			;@ Just change bits we know about.
;@	orr r0,r0,#0x8000			;@ PWM 7-bit 131.072kHz
;@	strh r0,[r5,#REG_SGBIAS]

	ldrb r4,soundMode
	cmp r4,#1					;@ If r4=0, no sound.

	movmi r0,#0
	ldreq r0,=0x0b040000		;@ Stop all channels, output ratio=100% dsA.  use directsound A for L&R, timer 0
	str r0,[r5,#REG_SGCNT_L]

	moveq r0,#0x80
	strh r0,[r5,#REG_SGCNT_X]	;@ Sound master enable

	mov r0,#0					;@ Triangle reset
	str r0,[r5,#REG_SG3CNT_L]	;@ Sound3 disable, mute, write bank 0

								;@ Mixer channels
	strh r5,[r5,#REG_DMA1CNT_H]	;@ DMA1 stop, SN76496
	add r0,r5,#REG_FIFO_A_L		;@ DMA1 destination..
	str r0,[r5,#REG_DMA1DAD]
	ldr r0,pcmPtr0
	str r0,[r5,#REG_DMA1SAD]	;@ DMA1 src=..

	cmp r4,#1					;@ If r4=0, no sound.

	mov r2,#0
	str r2,[r5,#REG_TM0CNT_L]	;@ Timer 0 controls sample rate. Stop timer 0
	moveq r1,#924				;@ 924=Low, 532=High.
	rsbeq r2,r1,#0x810000		;@ Timer 0 on. Frequency = 0x1000000/r3 Hz
	streq r2,[r5,#REG_TM0CNT_L]

	ldmfd sp!,{r4-r5,lr}
	bx lr

;@----------------------------------------------------------------------------
soundReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	ldr spxptr,=sphinx0
	bl wsAudioReset				;@ sound
	mov r0,#MIX_LEN
	ldr r1,=WAVBUFFER
	bl silenceMix
	mov r0,#0
	strb r0,muteSoundChip
	ldmfd sp!,{lr}
	bx lr

;@----------------------------------------------------------------------------
soundSetMuteGUI:
	.type   soundSetMuteGUI STT_FUNC
;@----------------------------------------------------------------------------
	ldr r1,=pauseEmulation		;@ Output silence when emulation paused.
	ldrb r0,[r1]
	strb r0,muteSoundGUI
	bx lr
;@----------------------------------------------------------------------------
soundSetMuteChip:
;@----------------------------------------------------------------------------
	strb r0,muteSoundChip
	bx lr
;@----------------------------------------------------------------------------
soundSwapBuffers:
	.type   soundSwapBuffers STT_FUNC
;@----------------------------------------------------------------------------
	ldrb r0,soundMode			;@ if r0=0, no sound.
	cmp r0,#0
	bxeq lr

	mov r1,#REG_BASE
	strh r1,[r1,#REG_DMA1CNT_H]	;@ DMA1 stop
	ldr r2,pcmPtr0
	str r2,[r1,#REG_DMA1SAD]	;@ DMA1 src=..
	ldr r0,=0xB640				;@ noIRQ fifo 32bit repeat incsrc fixeddst
	strh r0,[r1,#REG_DMA1CNT_H]	;@ DMA1 go

	ldr r1,pcmPtr1
	str r1,pcmPtr0
	str r2,pcmPtr1

	bx lr
;@----------------------------------------------------------------------------
soundRender:
	.type   soundRender STT_FUNC
;@----------------------------------------------------------------------------
	;@ update DMA buffer for PCM
	ldrb r0,soundMode			;@ if r0=0, no sound.
	cmp r0,#0
	bxeq lr

	mov r0,#MIX_LEN
	ldr r1,pcmPtr0
	ldr r2,muteSound
	cmp r2,#0

	ldreq spxptr,=sphinx0
	beq wsAudioMixer

;@----------------------------------------------------------------------------
silenceMix:					;@ r0=len, r1=destination
;@----------------------------------------------------------------------------
	mov r2,#0
silenceLoop:
	subs r0,r0,#4
	strpl r2,[r1],#4
	bhi silenceLoop

	bx lr
;@----------------------------------------------------------------------------
soundCopyBuffInt:			;@ Internal speaker, 8bit mono. r2=source
;@----------------------------------------------------------------------------
	stmfd sp!,{r5,lr}
	ldr lr,=0x80008000
	ldr r5,=0xFF00FF00
sndCpyIntLoop:
	subs r0,r0,#1
	ldrpl r3,[r2,r4,lsr#SHIFTVAL-2]
	add r4,r4,#1<<SHIFTVAL
	add r3,r3,r3,ror#16
	and r3,r3,r5
//mix8Vol:
	eor r3,lr,r3,lsr#0			;@ Volume button shift, updated by wsaSetTotalVolume
	strpl r3,[r1],#4
	bhi sndCpyIntLoop
	ldmfd sp!,{r5,pc}

;@----------------------------------------------------------------------------
pcmPtr0:	.long WAVBUFFER
pcmPtr1:	.long WAVBUFFER+MIX_LEN

muteSound:
muteSoundGUI:
	.byte 0
muteSoundChip:
	.byte 1
	.space 2
soundMode:
	.byte 0
	.space 3

#ifdef GBA
	.section .sbss				;@ This is EWRAM on GBA with devkitARM
#else
	.section .bss
#endif
	.align 2
//FREQTBL:
//	.space 1024*2
WAVBUFFER:
	.space MIX_LEN*2
;@----------------------------------------------------------------------------
	.end
#endif // __arm__
