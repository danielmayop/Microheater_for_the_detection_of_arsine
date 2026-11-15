
#ifndef DHT22_SENSOR_H
#define DHT22_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

class DHT22Sensor {

public:

    // ======================================================
    // CONSTRUCTOR
    // ======================================================

    DHT22Sensor(int pinDHT);


    // ======================================================
    // INICIALIZAR
    // ======================================================

    void iniciar();


    // ======================================================
    // LECTURAS
    // ======================================================

    float leerTemperatura();

    float leerHumedad();


    // ======================================================
    // ESTADO DEL SENSOR
    // ======================================================

    bool valido();


private:

    int _pinDHT;

    DHT* _dht;

    float _temperatura;

    float _humedad;

    bool _sensorOK;
};

#endif

