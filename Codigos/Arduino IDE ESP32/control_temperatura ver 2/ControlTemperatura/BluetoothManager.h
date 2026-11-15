#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <Arduino.h>
#include "BluetoothSerial.h"

class BluetoothManager {

public:
    BluetoothManager(const char* nombre);
    void iniciar();
    void conexion();
    void datos(float tiempo,float temperaturaReal,float temperaturaAproximada,float setpoint);                       
    void cerrar();

private:
    BluetoothSerial _bluetooth;
    const char* _nombre;
};

#endif