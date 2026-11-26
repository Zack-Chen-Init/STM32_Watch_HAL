#ifndef _MENU_H_
#define _MENU_H_

void Peripheral_Init(void);
void Show_Clock_UI(void);
int First_Page_Clock(void);


void Show_Setting_Page_UI(void);
int SettingPage(void);

int Menu(void);

void StopWatch_Tick(void);
int StopWatch(void);

int LED(); 

#endif