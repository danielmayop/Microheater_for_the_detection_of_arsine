#include "control.h"

bool calentadorOn = false;
uint8_t dutyActual = DUTY_OFF;

void inicializarControl() {
    ledcAttach(PWM_GPIO, PWM_FREQ, PWM_RES);
    apagarCalentador();
}

void encenderCalentador() {
    calentadorOn = true;
    dutyActual = DUTY_ON;
    ledcWrite(PWM_GPIO, dutyActual);
}

void apagarCalentador() {
    calentadorOn = false;
    dutyActual = DUTY_OFF;
    ledcWrite(PWM_GPIO, dutyActual);
}

void actualizarControl(float temperaturaReal, float setpoint) {
    if (temperaturaReal < setpoint - HISTERESIS) {
        encenderCalentador();
                  }
    else if (temperaturaReal >= setpoint) {
        apagarCalentador();
    }
    // Dentro de la histéresis conserva el estado anterior.
}