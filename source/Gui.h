#ifndef GUI_HEADER
#define GUI_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u8 gContrastValue;
extern u8 gBorderEnable;

void setupGUI(void);
void enterGUI(void);
void exitGUI(void);
void quickSelectGame(void);
void nullUINormal(int key);
void nullUIDebug(int key);
void resetGame(void);

void uiNullNormal(void);
void ui12(void);
void ui13(void);

void debugIOUnmappedR(u16 port);
void debugIOUnmappedW(u8 val, u16 port);
void debugIOUnimplR(u16 port);
void debugIOUnimplW(u8 val, u16 port);
void debugSerialOutW(u8 val);
void debugDivideError(void);
void debugUndefinedInstruction(void);
void debugCrashInstruction(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !GUI_HEADER
