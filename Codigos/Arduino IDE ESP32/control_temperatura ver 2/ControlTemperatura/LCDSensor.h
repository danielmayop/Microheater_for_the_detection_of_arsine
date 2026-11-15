#ifndef LCD_SENSOR_H
#define LCD_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display OLED SSD1306 128x64 por I2C.

class LCDSensor {

public:

    // Constructor (columnas y filas se ignoran, se mantienen por compatibilidad)
    LCDSensor(
        uint8_t direccion,
        uint8_t columnas,
        uint8_t filas
    );

    // Inicializar OLED (Wire.begin(SDA, SCL) debe llamarse antes)
    void iniciar();

    // PANEL COMPLETO: todos los datos juntos en una sola pantalla
    void mostrarPanel(
        float temperaturaNTC,
        float temperaturaAmbiente,
        float humedad,
        float setpoint,
        bool sistemaActivo,
        bool calentadorEncendido
    );

    // Pantallas individuales (se mantienen por compatibilidad)
    void mostrarNTC(float temperatura);

    void mostrarDHT22Ambiente(
        float temperatura,
        float humedad
    );

    void mostrarDHT22_2(
        float temperatura,
        float humedad
    );

    // Limpiar pantalla
    void limpiar();

    // Apagar / encender el panel fisicamente
    void apagar();
    void encender();


private:

    Adafruit_SSD1306* _lcd;

    uint8_t _direccion;
    uint8_t _columnas;
    uint8_t _filas;
    bool _ok;

    // Helpers
    void _pantallaDosValores(
        const char* titulo,
        float temperatura,
        float humedad
    );

    void _imprimirValor(
        float valor,
        const char* unidad,
        bool conGrado
    );

    void _celda(
        int16_t x,
        int16_t y,
        const char* etiqueta,
        float valor,
        const char* unidad,
        bool medicionActiva
    );
};

#endif