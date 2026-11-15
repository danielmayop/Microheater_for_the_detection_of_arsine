#include "DHT22Sensor.h"
#include <math.h>

DHT22Sensor::DHT22Sensor(int pinDHT) {
    _pinDHT = pinDHT;
    _dht = new DHT( _pinDHT, DHT22 );
    _temperatura = 0.0f;
    _humedad = 0.0f;
    _sensorOK = false;
}


// ======================================================
// INICIAR
// ======================================================

void DHT22Sensor::iniciar() {
    if (_dht == nullptr) {
        _sensorOK = false;

        return;
    }
    _dht->begin();  
    delay(2000);// El DHT22 necesita aproximadamente 2 segundos antes de realizar una primera lectura válida.
    leerTemperatura();
    leerHumedad();
}

// ======================================================
// LEER TEMPERATURA
// ======================================================

float DHT22Sensor::leerTemperatura() {

    if (_dht == nullptr) {

        _sensorOK = false;

        return NAN;
    }

    float temperatura = _dht->readTemperature();

    if (isnan(temperatura)) {
        _sensorOK = false;
        return NAN;
    }
    _temperatura = temperatura;
    _sensorOK = true;

    return _temperatura;
}


// ======================================================
// LEER HUMEDAD
// ======================================================

float DHT22Sensor::leerHumedad() {

    if (_dht == nullptr) {

        _sensorOK = false;

        return NAN;
    }


    float humedad = _dht->readHumidity();


    if (isnan(humedad)) {

        _sensorOK = false;

        return NAN;
    }


    _humedad = humedad;

    _sensorOK = true;


    return _humedad;
}


// ======================================================
// SENSOR VÁLIDO
// ======================================================

bool DHT22Sensor::valido() {

    return _sensorOK;
}
