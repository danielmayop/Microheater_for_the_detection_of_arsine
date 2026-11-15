#include "BluetoothDatos.h"
#include <math.h>

// UUIDs del servicio Nordic UART (NUS)
#define NUS_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define NUS_RX_UUID      "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // la página escribe aquí
#define NUS_TX_UUID      "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // el ESP32 notifica aquí

// Pausa entre paquetes para no saturar la pila BLE
static const unsigned long PAUSA_ENTRE_PAQUETES_MS = 5;


// ======================================================
// CALLBACKS DE BLE
// ======================================================

class CallbacksServidor : public BLEServerCallbacks {

public:
    CallbacksServidor(BluetoothDatos* dueno) : _dueno(dueno) {}

    void onConnect(BLEServer* servidor) {
        _dueno->_cambioConexion(true);
    }

    void onDisconnect(BLEServer* servidor) {
        _dueno->_cambioConexion(false);
    }

private:
    BluetoothDatos* _dueno;
};


class CallbacksRX : public BLECharacteristicCallbacks {

public:
    CallbacksRX(BluetoothDatos* dueno) : _dueno(dueno) {}

    void onWrite(BLECharacteristic* caracteristica) {

        // auto: funciona tanto con std::string (core 2.x) como con String (core 3.x)
        auto valor = caracteristica->getValue();

        String texto = "";

        for (size_t i = 0; i < valor.length(); i++) {
            texto += (char)valor[i];
        }

        texto.trim();

        if (texto.length() > 0) {
            _dueno->_comandoRecibido(texto);
        }
    }

private:
    BluetoothDatos* _dueno;
};


// ======================================================
// HELPERS PARA ARMAR EL JSON
// ======================================================

// Agrega  "clave":valor,  (o null si el valor no es válido)
static void agregarNumero(
    char* buffer,
    size_t tamano,
    size_t& pos,
    const char* clave,
    float valor
) {

    if (pos >= tamano - 1) return;

    int n;

    if (isnan(valor) || isinf(valor)) {

        n = snprintf(buffer + pos, tamano - pos, "\"%s\":null,", clave);
    }
    else {

        n = snprintf(buffer + pos, tamano - pos, "\"%s\":%.2f,", clave, valor);
    }

    if (n > 0) pos += n;
}


// Agrega  "clave":true/false,
static void agregarBooleano(
    char* buffer,
    size_t tamano,
    size_t& pos,
    const char* clave,
    bool valor
) {

    if (pos >= tamano - 1) return;

    int n = snprintf(buffer + pos, tamano - pos, "\"%s\":%s,", clave, valor ? "true" : "false");

    if (n > 0) pos += n;
}


// ======================================================
// CONSTRUCTOR
// ======================================================

BluetoothDatos::BluetoothDatos() {

    _servidor = nullptr;
    _tx = nullptr;
    _rx = nullptr;

    _conectado = false;
    _reiniciarPublicidad = false;

    _tamanoPaquete = 20;

    _comando = "";
    _hayComando = false;
}


// ======================================================
// INICIAR
// ======================================================

void BluetoothDatos::iniciar(const char* nombreDispositivo) {

    BLEDevice::init(nombreDispositivo);

    // Servidor
    _servidor = BLEDevice::createServer();
    _servidor->setCallbacks(new CallbacksServidor(this));

    // Servicio NUS
    BLEService* servicio = _servidor->createService(NUS_SERVICE_UUID);

    // TX: el ESP32 notifica datos a la página
    _tx = servicio->createCharacteristic(
        NUS_TX_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    _tx->addDescriptor(new BLE2902());   // necesario para las notificaciones

    // RX: la página puede enviar comandos
    _rx = servicio->createCharacteristic(
        NUS_RX_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    _rx->setCallbacks(new CallbacksRX(this));

    servicio->start();

    // Publicidad (para que la página pueda encontrar el dispositivo)
    BLEAdvertising* publicidad = BLEDevice::getAdvertising();
    publicidad->addServiceUUID(NUS_SERVICE_UUID);   // necesario para filtrar por servicio
    publicidad->setScanResponse(true);

    BLEDevice::startAdvertising();

    Serial.print("BLE listo. Nombre: ");
    Serial.println(nombreDispositivo);
}


// ======================================================
// ACTUALIZAR (reinicia la publicidad al desconectarse)
// ======================================================

void BluetoothDatos::actualizar() {

    if (_reiniciarPublicidad) {

        _reiniciarPublicidad = false;

        delay(300);   // da tiempo a la pila BLE a liberar la conexión

        BLEDevice::startAdvertising();

        Serial.println("BLE: publicidad reiniciada, esperando conexión...");
    }
}


// ======================================================
// ESTADO
// ======================================================

bool BluetoothDatos::conectado() const {

    return _conectado;
}

void BluetoothDatos::setTamanoPaquete(size_t tamano) {

    if (tamano < 20) tamano = 20;
    if (tamano > 500) tamano = 500;

    _tamanoPaquete = tamano;
}


// ======================================================
// CONSTRUIR JSON
// ======================================================

size_t BluetoothDatos::construirJSON(
    char* buffer,
    size_t tamano,
    float temperaturaReal,
    float temperaturaDHT,
    float humedad,
    float setpoint,
    bool calentador,
    bool sistemaActivo,
    float tiempo
) {

    if (buffer == nullptr || tamano < 8) return 0;

    size_t pos = 0;

    buffer[pos++] = '{';

    agregarNumero(buffer, tamano, pos, "temperatura_real", temperaturaReal);
    agregarNumero(buffer, tamano, pos, "temperatura_dht", temperaturaDHT);
    agregarNumero(buffer, tamano, pos, "humedad", humedad);
    agregarNumero(buffer, tamano, pos, "setpoint", setpoint);
    agregarBooleano(buffer, tamano, pos, "calentador", calentador);
    agregarBooleano(buffer, tamano, pos, "sistema", sistemaActivo);

    // Último campo: sin coma final
    if (pos < tamano - 1) {

        int n = snprintf(buffer + pos, tamano - pos, "\"tiempo\":%.2f", tiempo);

        if (n > 0) pos += n;
    }

    if (pos < tamano - 1) {

        buffer[pos++] = '}';
    }

    if (pos >= tamano) pos = tamano - 1;

    buffer[pos] = '\0';

    return pos;
}


// ======================================================
// ENVIAR TEXTO (fragmentado)
// ======================================================

bool BluetoothDatos::enviarTexto(const char* texto) {

    if (!_conectado || _tx == nullptr || texto == nullptr) return false;

    size_t total = strlen(texto);
    size_t enviado = 0;

    while (enviado < total) {

        size_t restante = total - enviado;
        size_t n = (restante < _tamanoPaquete) ? restante : _tamanoPaquete;

        _tx->setValue((uint8_t*)(texto + enviado), n);
        _tx->notify();

        enviado += n;

        delay(PAUSA_ENTRE_PAQUETES_MS);
    }

    return true;
}


// ======================================================
// ENVIAR DATOS (JSON + '\n')
// ======================================================

bool BluetoothDatos::enviarDatos(
    float temperaturaReal,
    float temperaturaDHT,
    float humedad,
    float setpoint,
    bool calentador,
    bool sistemaActivo,
    float tiempo
) {

    if (!_conectado) return false;

    char json[256];

    size_t largo = construirJSON(
        json,
        sizeof(json) - 2,    // deja lugar para '\n' y '\0'
        temperaturaReal,
        temperaturaDHT,
        humedad,
        setpoint,
        calentador,
        sistemaActivo,
        tiempo
    );

    json[largo] = '\n';      // el '\n' marca el final del mensaje
    json[largo + 1] = '\0';

    return enviarTexto(json);
}


// ======================================================
// COMANDOS RECIBIDOS
// ======================================================

bool BluetoothDatos::hayComando() const {

    return _hayComando;
}

String BluetoothDatos::leerComando() {

    String comando = _comando;

    _comando = "";
    _hayComando = false;

    return comando;
}


// ======================================================
// USO INTERNO (callbacks)
// ======================================================

void BluetoothDatos::_cambioConexion(bool conectado) {

    _conectado = conectado;

    if (conectado) {

        Serial.println("BLE: cliente conectado");
    }
    else {

        Serial.println("BLE: cliente desconectado");

        _reiniciarPublicidad = true;
    }
}

void BluetoothDatos::_comandoRecibido(const String& comando) {

    _comando = comando;
    _hayComando = true;
}