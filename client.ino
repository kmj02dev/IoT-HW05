#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLEAddress.h>
#include <BLEAdvertisedDevice.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-ab12-cd34-ef56-abcdef123456"

BLEClient* pClient;
BLERemoteCharacteristic* pRemoteCharacteristic;
bool doConnect = false;
BLEAdvertisedDevice* myDevice;

// 서버가 발견되었을 때 콜백
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Found Device: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      advertisedDevice.getScan()->stop();
    }
  }
};

// 서버에 연결하고 특성 가져오기
bool connectToServer() {
  Serial.println("Connecting to BLE Server...");

  pClient = BLEDevice::createClient();
  pClient->connect(myDevice);
 
  BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find service UUID.");
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID));
  if (pRemoteCharacteristic == nullptr) {
    Serial.println("Failed to find characteristic UUID.");
    pClient->disconnect();
    return false;
  }

  // 값 읽기
  if (pRemoteCharacteristic->canRead()) {
    String value = String(pRemoteCharacteristic->readValue().c_str());
    Serial.print("Received value: ");
    Serial.println(value);
  }

  // 값 쓰기 (클라이언트 → 서버 메시지 전송)
  if (pRemoteCharacteristic->canWrite()) {
    String message = "Hello from Client";
    pRemoteCharacteristic->writeValue(message.c_str(), message.length());
    Serial.println("Message sent to server.");
  }

  // 알림 설정
  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify([](BLERemoteCharacteristic* pCharacteristic, uint8_t* data, size_t length, bool isNotify) {
      Serial.print("Notification: ");
      for (size_t i = 0; i < length; i++) {
        Serial.print((char)data[i]);
      }
      Serial.println();
    });
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Client...");

  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10, false);
}

void loop() {
  if (doConnect) {
    if (connectToServer()) {
      Serial.println("Connected to server and characteristic ready.");
    } else {
      Serial.println("Connection failed.");
    }
    doConnect = false;
  }
  delay(1000);
}