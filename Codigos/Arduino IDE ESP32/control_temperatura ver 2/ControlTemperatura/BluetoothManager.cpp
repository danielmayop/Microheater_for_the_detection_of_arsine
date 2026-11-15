#include "BluetoothManager.h"


BluetoothManager::BluetoothManager(const char* nombre) {
    _nombre = nombre;
}

void BluetoothManager::iniciar() {
    _bluetooth.begin(_nombre);
    Serial.println("Bluetooth iniciado");
}
void BluetoothManager::conexion() {
    // Reservado para futuras funciones de conexión.
}

void BluetoothManager::datos(float tiempo,float temperaturaReal,float temperaturaAproximada,float setpoint) {                                                                                   
    _bluetooth.print(tiempo, 2);
    _bluetooth.print(",");
    _bluetooth.print(temperaturaReal, 2);
    _bluetooth.print(",");
    _bluetooth.print(temperaturaAproximada, 2);
    _bluetooth.print(",");
    _bluetooth.println(setpoint, 2);
}


void BluetoothManager::cerrar() {
    _bluetooth.end();
}