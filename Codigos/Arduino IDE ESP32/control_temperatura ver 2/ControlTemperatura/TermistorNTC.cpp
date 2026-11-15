#include "TermistorNTC.h"
#include <math.h>

// Tensión de alimentación del divisor (rail de 3V3 de la ESP32)
static const float VCC_MV = 3300.0f;

// Cantidad de muestras que se promedian en cada lectura
static const int MUESTRAS = 16;


// CONSTRUCTOR
TermistorNTC::TermistorNTC(int pinNTC, float resistenciaFija, float resistenciaNTC25, float temperaturaNTC25, float beta)
{
    _pinNTC = pinNTC;
    _temperatura = 0.0f;
    _resistenciaFija = resistenciaFija;
    _resistenciaNTC25 = resistenciaNTC25;
    _temperaturaNTC25 = temperaturaNTC25;
    _beta = beta;
}


// INICIAR
void TermistorNTC::iniciar() {
    pinMode(_pinNTC, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(_pinNTC, ADC_11db);   // rango completo ~0 a 3.3 V
}


// LEER TEMPERATURA
float TermistorNTC::leerTemperatura() {

    // Promedio de varias muestras en milivolts
    // (usa la calibración de fábrica del ADC de la ESP32, más lineal que analogRead)
    uint32_t acumulado = 0;

    for (int i = 0; i < MUESTRAS; i++) {
        acumulado += analogReadMilliVolts(_pinNTC);
    }

    float mv = (float)acumulado / MUESTRAS;

    // Evitar división por cero y valores fuera de rango (sensor abierto o en corto)
    if (mv <= 0.0f || mv >= VCC_MV) {
        return NAN;
    }


    // ==================================================
    // CALCULAR RESISTENCIA DEL NTC
    // ==================================================

    // Circuito:
    //
    //       3.3V
    //        |
    //       NTC
    //        |
    //        +------ ADC
    //        |
    //      R fija
    //        |
    //       GND
    //
    // V_adc = Vcc * R_fija / (R_ntc + R_fija)
    // => R_ntc = R_fija * (Vcc - V_adc) / V_adc

    float resistenciaNTC = _resistenciaFija * ((VCC_MV - mv) / mv);

    // Ecuación Beta
    float temperaturaKelvin = 1.0f / (
        (1.0f / (_temperaturaNTC25 + 273.15f)) +
        (1.0f / _beta) * logf(resistenciaNTC / _resistenciaNTC25)
    );

    _temperatura = temperaturaKelvin - 273.15f;   // Kelvin -> Celsius

    return _temperatura;
}