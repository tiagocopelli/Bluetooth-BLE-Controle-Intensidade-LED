/*
  ESP32 CLIENTE BLE - Controle de LED EXTERNO
*/
#include "BLEDevice.h"

// UUIDs (Precisam ser rigorosamente iguais aos do Servidor)
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID_1("beb5483e-36e1-4688-b7f5-ea07361b26a8"); 
static BLEUUID charUUID_2("e3223119-9445-4e96-a4a1-85358c4046a2"); 

// Pinos e configurações do LED
const int ledPin = 4;         
const int freq = 5000;        
// MUDANÇA: Resolução de 10 bits permite valores de 0 a 1023
const int resolution = 10;     

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;

static BLERemoteCharacteristic* pRemoteCharacteristic_1;
static BLERemoteCharacteristic* pRemoteCharacteristic_2;
static BLEAdvertisedDevice* myDevice;

// Callback 1: Recebe o contador do Servidor
static void notifyCallback_1(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (length == 4) {
    uint32_t value = *(uint32_t*)pData;
    // Opcional: Você pode comentar a linha abaixo se não quiser ver o contador no monitor serial
    // Serial.print("Contador: "); Serial.println(value);
  }
}

// Callback 2: Recebe o valor do App e aplica a nova lógica
static void notifyCallback_2(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  String valorRecebido = ""; 
  
  // Junta os caracteres recebidos
  for (int i = 0; i < length; i++) {
    valorRecebido += (char)pData[i];
  }
  
  Serial.print("App enviou o valor: ");
  Serial.println(valorRecebido);
  
  int brilho = valorRecebido.toInt();
  
  // NOVA LÓGICA:
  if (brilho < 50) {
    brilho = 0; // Se for menor que 50, apaga o LED completamente
  } else if (brilho > 1000) {
    brilho = 1000; // Se enviarem mais que 1000, trava no 1000
  }
  
  // Aplica o brilho ao LED
  ledcWrite(ledPin, brilho);
  
  Serial.print("Status do LED: ");
  if (brilho == 0) {
    Serial.println("APAGADO (< 50)");
  } else {
    Serial.print("ACESO (Brilho: ");
    Serial.print(brilho);
    Serial.println(")");
  }
  Serial.println("-------------------------");
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {}
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("Desconectado do servidor!");
  }
};

bool connectToServer() {
  Serial.print("Conectando ao Servidor...");
  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(myDevice);
  
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic_1 = pRemoteService->getCharacteristic(charUUID_1);
  if (pRemoteCharacteristic_1 != nullptr && pRemoteCharacteristic_1->canNotify()) {
    pRemoteCharacteristic_1->registerForNotify(notifyCallback_1);
  }

  pRemoteCharacteristic_2 = pRemoteService->getCharacteristic(charUUID_2);
  if (pRemoteCharacteristic_2 != nullptr && pRemoteCharacteristic_2->canNotify()) {
    pRemoteCharacteristic_2->registerForNotify(notifyCallback_2);
    Serial.println(" - Escuta ativada com sucesso!");
  }

  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop(); 
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true; 
      doScan = true;
    }
  }
};

void setup() {
  Serial.begin(115200);
  
  // Configura o PWM direto no Pino 4
  ledcAttach(ledPin, freq, resolution);
  // Garante que o LED inicie apagado
  ledcWrite(ledPin, 0);
  
  Serial.println("Iniciando Cliente BLE...");
  
  BLEDevice::init(""); 
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false); 
}

void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("Conexão estabelecida!");
    } else {
      Serial.println("Falha na conexão.");
    }
    doConnect = false;
  }

  if (connected) {
    delay(1000); 
  } else if (doScan) {
    BLEDevice::getScan()->start(0);
  }
}