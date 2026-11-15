#include "LecturaAnalogica.h"

LecturaAnalogica::LecturaAnalogica(int pinPotenciometro) {
    _pinPotenciometro = pinPotenciometro;
    pinMode(_pinPotenciometro, INPUT);
    analogReadResolution(12);
}

float LecturaAnalogica::SetearTemperatura(float temperaturaAmbiente) {
    int lecturaADC = analogRead(_pinPotenciometro);
    float proporcion = lecturaADC / 4095.0f;
    float setpoint =  temperaturaAmbiente + proporcion * (60.0f - temperaturaAmbiente);            
    return setpoint;
}