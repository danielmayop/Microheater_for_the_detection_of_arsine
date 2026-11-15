#ifndef TERMISTOR_NTC_H
#define TERMISTOR_NTC_H
#include <Arduino.h>

class TermistorNTC {

public:
    
    TermistorNTC( int pinNTC, float resistenciaFija,float resistenciaNTC25, float temperaturaNTC25, float beta);// Constructor
    void iniciar();// Inicialización
    float leerTemperatura();// Lectura

private:
    int _pinNTC;
    float _temperatura;
    // Parámetros del NTC
    float _resistenciaFija;
    float _resistenciaNTC25;
    float _temperaturaNTC25;
    float _beta;
};
#endif

