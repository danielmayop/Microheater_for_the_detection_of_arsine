#ifndef CONTROL_H
#define CONTROL_H
#include <Arduino.h>
#define PWM_GPIO 25 // antes 18
#define PWM_FREQ 1000
#define PWM_RES 8
#define DUTY_ON 255
#define DUTY_OFF 0
#define HISTERESIS 1.0f

extern bool calentadorOn;
extern uint8_t dutyActual;

void inicializarControl();
void actualizarControl(float temperaturaReal, float setpoint);
void encenderCalentador();
void apagarCalentador();

#endif