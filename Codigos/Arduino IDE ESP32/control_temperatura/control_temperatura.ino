#include <Arduino.h>
#include <max6675.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//=====================================================
// DEFINICION DE PINES
//=====================================================

#define PWM_GPIO      18
#define POTE_PIN      34

#define MAX6675_SCK   14
#define MAX6675_CS    13
#define MAX6675_SO    12

#define BOTON_PIN     26
#define LED_PIN       2

#define LCD_ADDR      0x27
#define LCD_COLS      16
#define LCD_ROWS      2

//=====================================================
// PARAMETROS PWM
//=====================================================

#define PWM_FREQ      1000
#define PWM_RES       8

//=====================================================
// RANGOS DE SETPOINT
//=====================================================

#define SP_MIN        30.0f
#define SP_MAX        60.0f

//=====================================================
// CONTROL BANG-BANG CON HISTERESIS
// Si T < (SP - HISTERESIS) → PWM ON (máximo)
// Si T >= SP                → PWM OFF
// Entre medio, mantiene el último estado (evita parpadeo)
//=====================================================

#define HISTERESIS    1.0f      // °C de banda muerta
#define DUTY_ON       255.0f
#define DUTY_OFF      0.0f

//=====================================================
// INTERVALOS DE TIEMPO
//=====================================================

#define INTERVALO_TEMP_MS    500
#define INTERVALO_CTRL_MS    500
#define INTERVALO_PRINT_MS   500
#define INTERVALO_LCD_MS     500
#define DEBOUNCE_MS          50

//=====================================================
// VARIABLES GLOBALES
//=====================================================

float dutyActual = 0.0f;
float tempReal  = 0.0f;
float setpoint  = 30.0f;
bool  sensorOk  = false;
bool  sistemaActivo = false;
bool  calentadorOn  = false;   // Estado del bang-bang

unsigned long ultimaLecturaTemp      = 0;
unsigned long ultimaImpresion        = 0;
unsigned long ultimaActualizacionLCD = 0;
unsigned long ultimoRebote           = 0;
unsigned long tAnteriorCtrl          = 0;

bool estadoBotonAnterior = HIGH;

//=====================================================
// OBJETOS
//=====================================================

MAX6675           termopar(MAX6675_SCK, MAX6675_CS, MAX6675_SO);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

//=====================================================
// PROTOTIPOS
//=====================================================

float leerTemperatura();
float leerSetpoint();
void  actualizarLCD();
void  leerBoton();
void  apagarSistema();
void  encenderSistema();

//=====================================================
// SETUP
//=====================================================

void setup()
{
    Serial.begin(115200);

    pinMode(BOTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    analogReadResolution(12);
    analogSetPinAttenuation(POTE_PIN, ADC_11db);

    ledcAttach(PWM_GPIO, PWM_FREQ, PWM_RES);
    ledcWrite(PWM_GPIO, 0);

    Wire.begin(16, 17);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("  Iniciando...  ");

    Serial.println("=================================");
    Serial.println("  Control Bang-Bang de temperatura");
    Serial.println("=================================");

    delay(1000);

    float tempInicial = termopar.readCelsius();

    if (isnan(tempInicial) || tempInicial == 0.0f)
    {
        sensorOk = false;
        Serial.println("[ERROR] Sensor no responde.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ERROR: Sensor");
        lcd.setCursor(0, 1);
        lcd.print("Verificar SPI");
    }
    else
    {
        tempReal = tempInicial;
        sensorOk = true;
        Serial.print("Temp inicial: ");
        Serial.print(tempReal, 1);
        Serial.println(" C | Sensor OK");
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sistema: OFF    ");
    lcd.setCursor(0, 1);
    lcd.print("Pulsa boton...  ");

    delay(500);
}

//=====================================================
// LOOP
//=====================================================

void loop()
{
    leerBoton();
    setpoint = leerSetpoint();

    //-------------------------------------------------
    // Leer temperatura SIEMPRE
    //-------------------------------------------------

    if (millis() - ultimaLecturaTemp >= INTERVALO_TEMP_MS)
    {
        ultimaLecturaTemp = millis();
        tempReal = leerTemperatura();
    }

    //-------------------------------------------------
    // LCD SIEMPRE
    //-------------------------------------------------

    if (millis() - ultimaActualizacionLCD >= INTERVALO_LCD_MS)
    {
        ultimaActualizacionLCD = millis();
        actualizarLCD();
    }

    if (!sistemaActivo) return;

    //-------------------------------------------------
    // Control Bang-Bang con histeresis
    //-------------------------------------------------

    if (millis() - tAnteriorCtrl >= INTERVALO_CTRL_MS)
    {
        tAnteriorCtrl = millis();

        if (sensorOk)
        {
            if (tempReal < (setpoint - HISTERESIS))
            {
                calentadorOn = true;
            }
            else if (tempReal >= setpoint)
            {
                calentadorOn = false;
            }
            // Si está entre (setpoint - HISTERESIS) y setpoint,
            // mantiene el último estado → evita parpadeo del relé/PWM

            dutyActual = calentadorOn ? DUTY_ON : DUTY_OFF;
        }
        else
        {
            dutyActual   = 0.0f;
            calentadorOn = false;
        }

        ledcWrite(PWM_GPIO, (uint32_t)dutyActual);
    }

    //-------------------------------------------------
    // Serial
    //-------------------------------------------------

    if (millis() - ultimaImpresion >= INTERVALO_PRINT_MS)
    {
        ultimaImpresion = millis();
        float dutyPct = (100.0f * dutyActual) / 255.0f;

        if (!sensorOk)
        {
            Serial.println("[ERROR] Sensor sin respuesta. PWM=0.");
        }
        else
        {
            Serial.print("SP: ");
            Serial.print(setpoint, 1);
            Serial.print(" C | T: ");
            Serial.print(tempReal, 1);
            Serial.print(" C | PWM: ");
            Serial.print(dutyPct, 1);
            Serial.print("% | Estado: ");
            Serial.println(calentadorOn ? "ON" : "OFF");
        }
    }
}

//=====================================================
// LEER SETPOINT
//=====================================================

float leerSetpoint()
{
    uint32_t suma = 0;
    for (int i = 0; i < 16; i++)
        suma += analogRead(POTE_PIN);

    int adc = suma / 16;
    return SP_MIN + (SP_MAX - SP_MIN) * (adc / 4095.0f);
}

//=====================================================
// LCD
// Línea 0: SP:XX.X T:XX.X°
// Línea 1: PWM:XXX% ON/OFF
//=====================================================

void actualizarLCD()
{
    char linea0[17];
    char linea1[17];

    if (!sistemaActivo)
    {
        snprintf(linea0, sizeof(linea0), "SP:%4.1f  OFF    ", setpoint);
        if (sensorOk)
            snprintf(linea1, sizeof(linea1), "T:%5.1f%cC      ", tempReal, (char)223);
        else
            snprintf(linea1, sizeof(linea1), "T: ---  ERROR   ");
    }
    else
    {
        if (sensorOk)
            snprintf(linea0, sizeof(linea0), "SP:%4.1f T:%4.1f%c", setpoint, tempReal, (char)223);
        else
            snprintf(linea0, sizeof(linea0), "SP:%4.1f T:ERR  ", setpoint);

        float dutyPct = (100.0f * dutyActual) / 255.0f;
        snprintf(linea1, sizeof(linea1), "PWM:%5.1f%% %-3s  ", dutyPct, calentadorOn ? "ON" : "OFF");
    }

    lcd.setCursor(0, 0);
    lcd.print(linea0);
    lcd.setCursor(0, 1);
    lcd.print(linea1);
}

//=====================================================
// FUNCIONES AUXILIARES
//=====================================================

void leerBoton()
{
    bool estadoActual = digitalRead(BOTON_PIN);

    if (estadoActual != estadoBotonAnterior)
    {
        if (millis() - ultimoRebote >= DEBOUNCE_MS)
        {
            ultimoRebote        = millis();
            estadoBotonAnterior = estadoActual;

            if (estadoActual == LOW)
                encenderSistema();
            else
                apagarSistema();
        }
    }
}

void encenderSistema()
{
    sistemaActivo = true;
    calentadorOn  = false;
    tAnteriorCtrl = millis();
    digitalWrite(LED_PIN, HIGH);
    Serial.println("[INFO] Sistema ENCENDIDO");
}

void apagarSistema()
{
    sistemaActivo = false;
    calentadorOn  = false;
    ledcWrite(PWM_GPIO, 0);
    dutyActual    = 0.0f;
    digitalWrite(LED_PIN, LOW);
    Serial.println("[INFO] Sistema APAGADO");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sistema: OFF    ");
    lcd.setCursor(0, 1);
    lcd.print("Pulsa boton...  ");
}

float leerTemperatura()
{
    float t = termopar.readCelsius();

    if (isnan(t))
    {
        Serial.println("[ERROR] Termopar desconectado o falla SPI.");
        sensorOk = false;
        return tempReal;
    }

    if (t == 0.0f)
    {
        Serial.println("[WARN] Lectura 0.0 C, posible falla de comunicacion.");
        sensorOk = false;
        return tempReal;
    }

    sensorOk = true;

    // Impresión de la temperatura leída por el sensor (siempre, sin importar el estado del sistema)
    Serial.print("[SENSOR] Temperatura leida: ");
    Serial.print(t, 1);
    Serial.println(" C");

    return t;
}