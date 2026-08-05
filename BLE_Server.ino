/*
  ESP32 SERVIDOR BLE (Multi-conexão + Notificação Bidirecional + Controle de LED)
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic_2 = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;

int clientCount = 0; // Quantidade de dispositivos conectados
uint32_t value = 0;  // Contador infinito

// Pinos e configurações do LED no SERVIDOR (API Core v3.x)
const int ledPin = 4;         // Pino 4 para o LED Externo no Servidor
const int freq = 5000;        // Frequência
const int resolution = 10;    // Resolução de 10 bits (0 a 1023)

// UUIDs da nossa "Rede"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR1_UUID          "beb5483e-36e1-4688-b7f5-ea07361b26a8" // Notifica o contador
#define CHAR2_UUID          "e3223119-9445-4e96-a4a1-85358c4046a2" // Lê/Escreve e Notifica o App

// Controle de Conexão Geral
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      clientCount++;
      Serial.print("Dispositivo conectado! Clientes ativos: ");
      Serial.println(clientCount);
      // Retoma a propaganda para o próximo dispositivo conseguir achar
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      clientCount--;
      Serial.print("Dispositivo desconectado! Clientes ativos: ");
      Serial.println(clientCount);
      if (clientCount == 0) {
        delay(500); 
        pServer->startAdvertising(); 
        Serial.println("Servidor vazio. Reiniciando advertising...");
      }
    }
};

// Controle da Característica 2 (Quando o celular envia o comando de brilho)
class CharacteristicCallBack: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override { 
    // Captura o valor e converte de forma segura para String do Arduino
    String pChar2_value_string = pChar->getValue().c_str();                
    
    Serial.print("App enviou o valor: ");
    Serial.println(pChar2_value_string); 
    
    // Converte para número inteiro
    int brilho = pChar2_value_string.toInt();
    
    // LÓGICA DE LIMIAR (Threshold)
    if (brilho < 50) {
      brilho = 0;    // Apaga completamente
    } else if (brilho > 1000) {
      brilho = 1000; // Trava no brilho máximo estabelecido
    }
    
    // Aplica o brilho ao LED físico conectado a este ESP32 Servidor
    ledcWrite(ledPin, brilho);
    
    Serial.print("Status do LED (Servidor): ");
    if (brilho == 0) {
      Serial.println("APAGADO (< 50)");
    } else {
      Serial.print("ACESO (Brilho: ");
      Serial.print(brilho);
      Serial.println(")");
    }
    Serial.println("-------------------------");
    
    // RETRANSMITE: Avisa imediatamente os outros clientes (como o seu ESP32 Cliente) sobre o novo valor
    pChar->notify();
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando Servidor BLE...");

  // Configura o PWM do LED no Servidor
  ledcAttach(ledPin, freq, resolution);
  ledcWrite(ledPin, 0); // Garante que inicie apagado

  BLEDevice::init("ESP32_Server");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Característica 1 (Apenas NOTIFY)
  pCharacteristic = pService->createCharacteristic(
                      CHAR1_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );                   

  // Característica 2 (READ, WRITE e NOTIFY)
  pCharacteristic_2 = pService->createCharacteristic(
                      CHAR2_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY // Suporte para notificar os clientes
                    );  

  // Descritores obrigatórios para o Notify funcionar no GATT
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic_2->addDescriptor(new BLE2902());
  
  // Associa a rotina de recebimento de dados
  pCharacteristic_2->setCallbacks(new CharacteristicCallBack());
  
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  
  BLEDevice::startAdvertising();
  
  Serial.println("Servidor pronto! Aguardando conexões...");
}

void loop() {
    // A cada 1 segundo, envia o contador para todos os dispositivos conectados
    if (clientCount > 0) {
        pCharacteristic->setValue(value);
        pCharacteristic->notify(); 
        value++;
        delay(1000); // Aguarda 1 segundo
    }
}