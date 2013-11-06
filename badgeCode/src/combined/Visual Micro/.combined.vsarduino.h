#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
//Board = Arduino Uno
#define __AVR_ATmega328P__
#define 
#define ARDUINO 105
#define ARDUINO_MAIN
#define __AVR__
#define F_CPU 16000000L
#define __cplusplus
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__

#define __builtin_va_list
#define __builtin_va_start
#define __builtin_va_end
#define __DOXYGEN__
#define __attribute__(x)
#define NOINLINE __attribute__((noinline))
#define prog_void
#define PGM_VOID_P int
            
typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

void EEPROMWriteInt(int p_address, int p_value);
unsigned int EEPROMReadInt(int p_address);
//
void showAbout();
void showSchedule();
void badgeIntro();
void MenuScreen();
void loadTopHeader(char* text);
void showStats();
void MainMenu();
void showCurrentSpeaker();
void screenScroll();
void showLineup();
void parseCmds(uint8_t* buf,int buflen);
void showFreeMem();
int readButtons();
void setLED(int red,int green,int blue);
void LED_OFF();
void LED_RED();
void LED_GREEN();
void LED_BLUE();
void LED_PURPLE();
void LED_WHITE();
//

#include "C:\Program Files (x86)\Arduino\hardware\arduino\variants\standard\pins_arduino.h" 
#include "C:\Program Files (x86)\Arduino\hardware\arduino\cores\arduino\arduino.h"
#include "c:\pub\ZACONV\combined\combined.ino"
#include "c:\pub\ZACONV\combined\zaconlogo.c"
#endif
