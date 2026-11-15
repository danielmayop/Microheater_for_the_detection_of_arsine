#ifndef LECTURA_ANALOGICA_H
#define LECTURA_ANALOGICA_H

#include <Arduino.h>

class LecturaAnalogica {

public:
    LecturaAnalogica(int pinPotenciometro);
    float SetearTemperatura(float temperaturaAmbiente);

private:
    int _pinPotenciometro;
};

#endif