#ifndef SOUND_HEADER
#define SOUND_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u8 soundMode;

void soundInit(void);
void soundSetMuteGUI(void);
void soundSwapBuffers(void);
void soundRender(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !SOUND_HEADER
