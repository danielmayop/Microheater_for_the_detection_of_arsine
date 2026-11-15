#include <Arduino.h>
#include <Wire.h>
#include "LecturaAnalogica.h"
#include "TermistorNTC.h"
#include "DHT22Sensor.h"
#include "LCDSensor.h"
#include "control.h"
#include "BluetoothDatos.h"


#define PIN_POT 34
#define PIN_INTERRUPTOR 26
#define PIN_LED 2
#define PIN_NTC 35
#define PIN_DHT22 18
#define PIN_LCD_SDA 16// RX2 = GPIO16 -> SDA
#define PIN_LCD_SCL 17// TX2 = GPIO17 -> SCL
const float TEMPERATURA_MAX_SEGURA = 80.0f;   // °C: ajustá según tu calentador
// CONFIGURACIÓN DEL TERMISTOR
// EJEMPLO PARA NTC 10 kΩ / Beta 3950
// Si tu termistor tiene otros valores, modificarlos.
const float RESISTENCIA_FIJA = 10000.0f;
const float RESISTENCIA_NTC25 = 10000.0f;
const float TEMPERATURA_NTC25 = 25.0f;
const float BETA_NTC = 3950.0f;

// VARIABLES DE TEMPERATURA
float temperaturaReal = NAN;        // Temperatura medida por el NTC (solo con sistema ON)
float temperaturaSetpoint = 25.0f;  // Temperatura deseada (potenciómetro, siempre activo)
float temperaturaDHT = NAN;         // Temperatura medida por el DHT22 (solo con sistema ON)
float humedadDHT = NAN;             // Humedad medida por el DHT22 (solo con sistema ON)

// Última temperatura ambiente válida del DHT22.
// Se usa solo como referencia para el potenciómetro, que siempre está activo.
float temperaturaAmbienteRef = NAN;

// ESTADO DEL SISTEMA (INTERRUPTOR)
bool sistemaActivo = false;
bool sistemaActivoAnterior = false;

// TIEMPOS
unsigned long tiempoAnteriorDHT = 0;      // Lectura DHT22
unsigned long tiempoAnteriorPantalla = 0; // Refresco del OLED
unsigned long tiempoAnteriorSerial = 0;   // Serial
unsigned long tiempoAnteriorBLE = 0;      // Transmisión Bluetooth
unsigned long tiempoInicioSistema = 0;    // Momento en que se pasó el interruptor a ON

// CONFIGURACIÓN DE TIEMPOS
const unsigned long INTERVALO_DHT = 2000;      // DHT22: cada 2 segundos
const unsigned long INTERVALO_PANTALLA = 250;  // OLED: refresco 4 veces por segundo
const unsigned long INTERVALO_SERIAL = 1000;   // Serial: cada 1 segundo
const unsigned long INTERVALO_BLE = 1000;      // Bluetooth: cada 1 segundo

// OBJETOS
LecturaAnalogica entradas(PIN_POT);                                                             // Potenciómetro
TermistorNTC termistor(PIN_NTC,RESISTENCIA_FIJA,RESISTENCIA_NTC25, TEMPERATURA_NTC25, BETA_NTC);// Termistor NTC
DHT22Sensor dht22(PIN_DHT22);                                                                   // DHT22
LCDSensor lcd(0x3C,16,2);                                                                       // OLED SSD1306
BluetoothDatos ble;                                                                             // Bluetooth BLE


// IMPRIMIR UNA MEDICIÓN POR SERIAL
void imprimirMedicion(const char* etiqueta, float valor, const char* unidad) {

    Serial.print(etiqueta);

    if (!sistemaActivo) {
        Serial.println("OFF (sin medicion)");
    }
    else if (isnan(valor)) {
        Serial.println("ERROR");
    }
    else {
        Serial.print(valor, 2);
        Serial.print(" ");
        Serial.println(unidad);
    }
}


// MOSTRAR DATOS POR SERIAL
void mostrarDatos() {

    Serial.println("--------------------------------");

    Serial.print("Interruptor (D26): ");
    Serial.println(sistemaActivo ? "ON" : "OFF");

    imprimirMedicion("Temperatura NTC: ", temperaturaReal, "°C");

    Serial.print("Temperatura seteada: ");

    if (isnan(temperaturaSetpoint)) {
        Serial.println("ERROR");
    }
    else {
        Serial.print(temperaturaSetpoint, 2);
        Serial.println(" °C");
    }

    imprimirMedicion("Temperatura DHT22: ", temperaturaDHT, "°C");
    imprimirMedicion("Humedad DHT22: ", humedadDHT, "%");

    Serial.print("Calentador: ");
    Serial.println(calentadorOn ? "ON" : "OFF");

    Serial.print("Duty: ");
    Serial.println(dutyActual);

    Serial.print("Bluetooth: ");

    if (!ble.conectado()) {
        Serial.println("sin cliente conectado");
    }
    else if (sistemaActivo) {
        Serial.println("conectado, transmitiendo");
    }
    else {
        Serial.println("conectado, transmision en pausa (sistema OFF)");
    }
}


// ACTUALIZAR OLED (panel completo, siempre encendido)
void actualizarPantalla() {

    lcd.mostrarPanel(
        temperaturaReal,
        temperaturaDHT,
        humedadDHT,
        temperaturaSetpoint,
        sistemaActivo,
        calentadorOn
    );
}


// APLICAR CAMBIO DE ESTADO DEL INTERRUPTOR
void aplicarEstadoSistema(bool activo) {

    if (activo) {                       // SISTEMA ENCENDIDO
        digitalWrite(PIN_LED, HIGH);

        // Forzar una lectura inmediata del DHT22 al encender
        tiempoAnteriorDHT = millis() - INTERVALO_DHT;

        // Reiniciar el tiempo de transmisión: el tiempo empieza en 0 al encender
        tiempoInicioSistema = millis();
        tiempoAnteriorBLE = millis();
    }
    else {                              // SISTEMA APAGADO
        apagarCalentador();
        digitalWrite(PIN_LED, LOW);

        // Descartar las mediciones: ya no se actualizan
        temperaturaReal = NAN;
        temperaturaDHT = NAN;
        humedadDHT = NAN;
    }
}


void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("=================================");
    Serial.println(" SISTEMA MICROCALENTADOR ESP32 ");
    Serial.println("=================================");

    pinMode(PIN_INTERRUPTOR, INPUT_PULLUP);   // INTERRUPTOR (ON = D26 a GND)
    pinMode(PIN_LED, OUTPUT);                 // LED
    digitalWrite(PIN_LED, LOW);
    Wire.begin(PIN_LCD_SDA, PIN_LCD_SCL);     // I2C
    Wire.setClock(400000);                    // I2C rápido: refresco del OLED más fluido
    termistor.iniciar();                      // TERMISTOR
    dht22.iniciar();                          // DHT22
    lcd.iniciar();                            // OLED
    inicializarControl();                     // CONTROL DEL CALENTADOR
    apagarCalentador();                       // ARRANQUE SEGURO
    ble.iniciar("Microcalentador-ESP32");     // BLUETOOTH

    // LECTURA INICIAL DEL DHT22 (referencia para el potenciómetro)
    temperaturaDHT = dht22.leerTemperatura();
    humedadDHT = dht22.leerHumedad();

    if (!isnan(temperaturaDHT)) {
        temperaturaAmbienteRef = temperaturaDHT;
    }

    // PRIMER SETPOINT
    if (!isnan(temperaturaAmbienteRef)) {
        temperaturaSetpoint = entradas.SetearTemperatura(temperaturaAmbienteRef);
    }

    // ESTADO INICIAL SEGÚN EL INTERRUPTOR
    sistemaActivo = (digitalRead(PIN_INTERRUPTOR) == LOW);
    sistemaActivoAnterior = sistemaActivo;

    if (sistemaActivo) {
        temperaturaReal = termistor.leerTemperatura();
    }

    aplicarEstadoSistema(sistemaActivo);

    // INICIALIZAR TEMPORIZADORES
    tiempoAnteriorDHT = millis();
    tiempoAnteriorPantalla = millis();
    tiempoAnteriorSerial = millis();
    tiempoAnteriorBLE = millis();

    if (sistemaActivo) {
        tiempoAnteriorDHT = millis() - INTERVALO_DHT;  // leer el DHT22 enseguida
        tiempoInicioSistema = millis();
    }

    actualizarPantalla();

    Serial.println("Sistema listo.");
}

void loop() {

    // BLUETOOTH: mantener la publicidad activa si se desconecta el cliente
    ble.actualizar();

    // LEER INTERRUPTOR Y DETECTAR CAMBIO
    sistemaActivo = (digitalRead(PIN_INTERRUPTOR) == LOW);

    if (sistemaActivo != sistemaActivoAnterior) {
        sistemaActivoAnterior = sistemaActivo;
        aplicarEstadoSistema(sistemaActivo);
        Serial.println(sistemaActivo ? ">>> SISTEMA ON" : ">>> SISTEMA OFF");
    }

    unsigned long ahora = millis();

    // MEDICIONES (SOLO CON EL SISTEMA ENCENDIDO)
    if (sistemaActivo) {

        // TERMISTOR
        temperaturaReal = termistor.leerTemperatura();

        // DHT22
        if (ahora - tiempoAnteriorDHT >= INTERVALO_DHT) {
            tiempoAnteriorDHT = ahora;
            temperaturaDHT = dht22.leerTemperatura();
            humedadDHT = dht22.leerHumedad();

            if (!isnan(temperaturaDHT)) {
                temperaturaAmbienteRef = temperaturaDHT;
            }
        }
    }

    // POTENCIÓMETRO: SIEMPRE ACTIVO
    if (!isnan(temperaturaAmbienteRef)) {
        temperaturaSetpoint = entradas.SetearTemperatura(temperaturaAmbienteRef);
    }

    // CONTROL DEL CALENTADOR
    if (!sistemaActivo) {                 // SISTEMA APAGADO
        apagarCalentador();
        digitalWrite(PIN_LED, LOW);
    }
    else if (isnan(temperaturaReal) || temperaturaReal > TEMPERATURA_MAX_SEGURA) {   // NTC INVÁLIDO O SOBRETEMPERATURA
        apagarCalentador();
        digitalWrite(PIN_LED, LOW);
    }
    else {                                // SISTEMA ENCENDIDO
        digitalWrite(PIN_LED, HIGH);
        actualizarControl(temperaturaReal, temperaturaSetpoint);
    }

    // BLUETOOTH: TRANSMITIR SOLO CON EL SISTEMA ENCENDIDO
    if (sistemaActivo && ble.conectado() && (ahora - tiempoAnteriorBLE >= INTERVALO_BLE)) {
        tiempoAnteriorBLE = ahora;

        ble.enviarDatos(
            temperaturaReal,                                  // NTC
            temperaturaDHT,
            humedadDHT,
            temperaturaSetpoint,
            calentadorOn,
            sistemaActivo,
            (ahora - tiempoInicioSistema) / 1000.0f           // segundos desde que se encendió
        );
    }

    // OLED: SIEMPRE ENCENDIDO
    if (ahora - tiempoAnteriorPantalla >= INTERVALO_PANTALLA) {
        tiempoAnteriorPantalla = ahora;
        actualizarPantalla();
    }

    // SERIAL
    if (ahora - tiempoAnteriorSerial >= INTERVALO_SERIAL) {
        tiempoAnteriorSerial = ahora;
        mostrarDatos();
    }

    delay(20);// PEQUEÑA PAUSA DEL LOOP
}