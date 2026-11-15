#include "LCDSensor.h"

#define OLED_ANCHO 128
#define OLED_ALTO  64


// ======================================================
// CONSTRUCTOR
// ======================================================

LCDSensor::LCDSensor(
    uint8_t direccion,
    uint8_t columnas,
    uint8_t filas
) {

    _direccion = direccion;

    _columnas = columnas;

    _filas = filas;

    _ok = false;

    _lcd = new Adafruit_SSD1306(
        OLED_ANCHO,
        OLED_ALTO,
        &Wire,
        -1              // sin pin de reset
    );
}


// ======================================================
// INICIAR
// ======================================================

void LCDSensor::iniciar() {

    _ok = _lcd->begin(SSD1306_SWITCHCAPVCC, _direccion);

    if (!_ok) {

        Serial.println("ERROR: no se pudo iniciar el OLED (revisar direccion y cableado)");

        return;
    }

    _lcd->clearDisplay();

    _lcd->setTextColor(SSD1306_WHITE);

    _lcd->display();
}


// ======================================================
// HELPER: UNA CELDA (etiqueta chica + valor grande)
// ======================================================

void LCDSensor::_celda(
    int16_t x,
    int16_t y,
    const char* etiqueta,
    float valor,
    const char* unidad,
    bool medicionActiva
) {

    // Etiqueta
    _lcd->setTextSize(1);
    _lcd->setCursor(x, y);
    _lcd->print(etiqueta);

    // Valor
    _lcd->setTextSize(2);
    _lcd->setCursor(x, y + 9);

    if (!medicionActiva) {

        _lcd->print("OFF");
    }
    else if (isnan(valor)) {

        _lcd->print("ERR");
    }
    else {

        // Sin decimales si el valor es >= 100 para que entre en la celda
        _lcd->print(valor, (fabsf(valor) >= 100.0f) ? 0 : 1);

        _lcd->print(unidad);
    }
}


// ======================================================
// PANEL COMPLETO
// ======================================================

void LCDSensor::mostrarPanel(
    float temperaturaNTC,
    float temperaturaAmbiente,
    float humedad,
    float setpoint,
    bool sistemaActivo,
    bool calentadorEncendido
) {

    if (!_ok) return;

    _lcd->clearDisplay();

    _lcd->setTextColor(SSD1306_WHITE);


    // Encabezado: estado del sistema y del calentador

    _lcd->setTextSize(1);

    _lcd->setCursor(0, 0);
    _lcd->print("SISTEMA:");
    _lcd->print(sistemaActivo ? "ON" : "OFF");

    _lcd->setCursor(72, 0);
    _lcd->print("CAL:");
    _lcd->print(calentadorEncendido ? "ON" : "OFF");


    // Lineas separadoras

    _lcd->drawFastHLine(0, 9, OLED_ANCHO, SSD1306_WHITE);
    _lcd->drawFastHLine(0, 36, OLED_ANCHO, SSD1306_WHITE);
    _lcd->drawFastVLine(63, 11, 53, SSD1306_WHITE);


    // Celdas: NTC y setpoint arriba, DHT22 abajo

    _celda(0,  11, "NTC",      temperaturaNTC,      "C", sistemaActivo);
    _celda(66, 11, "SETPOINT", setpoint,            "C", true);          // siempre visible
    _celda(0,  38, "AMBIENTE", temperaturaAmbiente, "C", sistemaActivo);
    _celda(66, 38, "HUMEDAD",  humedad,             "%", sistemaActivo);

    _lcd->display();
}


// ======================================================
// HELPER: IMPRIMIR VALOR (o ERR si es NaN)
// ======================================================

void LCDSensor::_imprimirValor(
    float valor,
    const char* unidad,
    bool conGrado
) {

    if (isnan(valor)) {

        _lcd->print("ERR");

    } else {

        _lcd->print(valor, 1);

        if (conGrado) {

            _lcd->write(248);   // simbolo de grado (CP437)
        }

        _lcd->print(unidad);
    }
}


// ======================================================
// HELPER: PANTALLA CON TITULO + TEMPERATURA + HUMEDAD
// ======================================================

void LCDSensor::_pantallaDosValores(
    const char* titulo,
    float temperatura,
    float humedad
) {

    _lcd->clearDisplay();

    _lcd->setTextColor(SSD1306_WHITE);

    _lcd->setTextSize(1);
    _lcd->setCursor(0, 0);
    _lcd->print(titulo);

    _lcd->setTextSize(2);
    _lcd->setCursor(0, 20);
    _lcd->print("T:");
    _imprimirValor(temperatura, "C", true);

    _lcd->setCursor(0, 44);
    _lcd->print("H:");
    _imprimirValor(humedad, "%", false);

    _lcd->display();
}


// ======================================================
// MOSTRAR NTC
// ======================================================

void LCDSensor::mostrarNTC(float temperatura) {

    if (!_ok) return;

    _lcd->clearDisplay();

    _lcd->setTextColor(SSD1306_WHITE);

    _lcd->setTextSize(1);
    _lcd->setCursor(0, 0);
    _lcd->print("NTC");

    _lcd->setTextSize(3);
    _lcd->setCursor(0, 26);

    if (isnan(temperatura)) {

        _lcd->print("ERROR");

    } else {

        _lcd->print(temperatura, 1);
        _lcd->write(248);
        _lcd->print("C");
    }

    _lcd->display();
}


// ======================================================
// MOSTRAR DHT22 AMBIENTE
// ======================================================

void LCDSensor::mostrarDHT22Ambiente(
    float temperatura,
    float humedad
) {

    if (!_ok) return;

    _pantallaDosValores("Ambiente (DHT22)", temperatura, humedad);
}


// ======================================================
// MOSTRAR SEGUNDO DHT22
// ======================================================

void LCDSensor::mostrarDHT22_2(
    float temperatura,
    float humedad
) {

    if (!_ok) return;

    _pantallaDosValores("DHT22 #2", temperatura, humedad);
}


// ======================================================
// LIMPIAR
// ======================================================

void LCDSensor::limpiar() {

    if (!_ok) return;

    _lcd->clearDisplay();

    _lcd->display();
}


// ======================================================
// APAGAR
// ======================================================

void LCDSensor::apagar() {

    if (!_ok) return;

    _lcd->clearDisplay();

    _lcd->display();

    _lcd->ssd1306_command(SSD1306_DISPLAYOFF);
}


// ======================================================
// ENCENDER
// ======================================================

void LCDSensor::encender() {

    if (!_ok) return;

    _lcd->ssd1306_command(SSD1306_DISPLAYON);
}