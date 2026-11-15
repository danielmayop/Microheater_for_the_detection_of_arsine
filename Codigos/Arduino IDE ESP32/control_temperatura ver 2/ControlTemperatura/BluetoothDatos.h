#ifndef BLUETOOTH_DATOS_H
#define BLUETOOTH_DATOS_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Transmisión de datos por BLE usando el servicio Nordic UART (NUS).
// Los datos se envían como JSON en una línea terminada en '\n'.

class BluetoothDatos {

public:

    BluetoothDatos();

    // Inicia BLE y empieza a publicitarse (llamar una vez en setup)
    void iniciar(const char* nombreDispositivo);

    // Llamar en cada vuelta del loop: reinicia la publicidad tras una desconexión
    void actualizar();

    // ¿Hay un cliente conectado?
    bool conectado() const;

    // Arma el JSON en un buffer (útil también para imprimirlo por Serial)
    // Devuelve la cantidad de caracteres escritos.
    size_t construirJSON(
        char* buffer,
        size_t tamano,
        float temperaturaReal,   // termistor NTC
        float temperaturaDHT,
        float humedad,
        float setpoint,
        bool calentador,
        bool sistemaActivo,
        float tiempo             // segundos
    );

    // Arma el JSON y lo envía por BLE. Devuelve false si no hay cliente conectado.
    bool enviarDatos(
        float temperaturaReal,
        float temperaturaDHT,
        float humedad,
        float setpoint,
        bool calentador,
        bool sistemaActivo,
        float tiempo
    );

    // Envía un texto cualquiera (se fragmenta en paquetes)
    bool enviarTexto(const char* texto);

    // Tamaño de cada paquete BLE en bytes (20 es seguro con cualquier cliente)
    void setTamanoPaquete(size_t tamano);

    // Comandos recibidos desde la página (característica RX)
    bool hayComando() const;
    String leerComando();   // devuelve el comando y lo borra

    // Uso interno (llamados por los callbacks de BLE)
    void _cambioConexion(bool conectado);
    void _comandoRecibido(const String& comando);

private:

    BLEServer* _servidor;
    BLECharacteristic* _tx;
    BLECharacteristic* _rx;

    volatile bool _conectado;
    volatile bool _reiniciarPublicidad;

    size_t _tamanoPaquete;

    String _comando;
    volatile bool _hayComando;
};

#endif