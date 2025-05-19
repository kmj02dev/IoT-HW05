#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-ab12-cd34-ef56-abcdef123456"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// 클라이언트 → 서버 데이터 수신
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxRaw = pCharacteristic->getValue();
    String rxValue = String(rxRaw.c_str());  // Arduino String으로 변환
    if (rxValue.length() > 0) {
      Serial.print("📥 From Client: ");
      Serial.println(rxValue);
    }
  }
};

// 연결 상태 추적
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("✅ Client connected.");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("❌ Client disconnected.");
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("ESP32_BLE_Server");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("🚀 BLE Server advertising...");
}

void loop() {
  // 사용자가 Serial에 입력하면 → 클라이언트에게 전송
  if (deviceConnected && Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    pCharacteristic->setValue(msg.c_str());
    pCharacteristic->notify();
    Serial.print("📤 Sent to Client: ");
    Serial.println(msg);
  }
}