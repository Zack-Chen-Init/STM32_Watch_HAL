#ifndef __LED_H
#define __LED_H

void LED_Init(void);
void LED_ON(void);
void LED_OFF(void);
void LED_Turn(void);

void LED_SOS();
extern uint8_t led_sos_flag;

#endif
