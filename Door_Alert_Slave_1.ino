#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Buzzer.h>

Buzzer buzzer(4);
#define RETRY_INTERVAL 5000
boolean doorState = false;
boolean alertFlag = false;
unsigned long timeReceived = 0;

// the following 3 settings must match transmitter's settings
uint8_t mac[] = {0x82, 0x88, 0x88, 0x88, 0x88, 0x88};
const uint8_t channel = 14;
struct __attribute__((packed)) DataStruct {
  boolean isDoorOpen;
};

DataStruct statusData;

void receiveCallBackFunction(uint8_t *senderMac, uint8_t *incomingData, uint8_t len) {
  doorState = true;
  timeReceived = millis();
  memcpy(&statusData, incomingData, len);
  Serial.printf("Transmitter MacAddr: %02x:%02x:%02x:%02x:%02x:%02x, ", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);
  Serial.printf("Is Door Open?: %0d ", statusData.isDoorOpen);
  Serial.println();
}

void setup() {
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(D5, INPUT);

  WiFi.mode(WIFI_AP);
  wifi_set_macaddr(SOFTAP_IF, &mac[0]);
  WiFi.disconnect();

  Serial.begin(115200);
  Serial.println();
  Serial.println("ESP-Now Receiver");
  Serial.printf("Transmitter mac: %s\n", WiFi.macAddress().c_str());
  Serial.printf("Receiver mac: %s\n", WiFi.softAPmacAddress().c_str());
  if (esp_now_init() != 0) {
    Serial.println("ESP_Now init failed...");
    delay(RETRY_INTERVAL);
    ESP.restart();
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(receiveCallBackFunction);
  Serial.println("Slave ready. Waiting for messages...");
}

void loop() {
  Serial.print("DOOR IS ");
  Serial.println((doorState ? "OPEN" : "CLOSED"));
  if (millis() - timeReceived > 150)
    doorState = false;

  if (doorState && alertFlag) {
    alert();
    alertFlag = false;
  }

  if (!doorState && !alertFlag) {
    alert();
    alertFlag = true;
  }
  /*
    Serial.println(statusData.isDoorOpen);
    delay(100);
  */
}

void alert() {
  buzzer.begin(5);
  for (int i = 0; i < 5; i++) {
    buzzer.sound(NOTE_GS4, 200);
    digitalWrite(5, HIGH);
    delay(500);
    buzzer.sound(NOTE_G  ;
    delay(500);
  }
  buzzer.end(5);
  /*
    for(int i = 0; i < 10; i++){
    buzzer.sound(NOTE_GS4, 200);
    delay(100);
    }
    buzzer.sound(NOTE_F5, 5000);
    buzzer.sound(NOTE_G5, 5000);
    buzzer.sound(NOTE_A5, 5000);
    buzzer.sound(NOTE_F4, 5000);
    buzzer.sound(NOTE_G4, 5000);
    buzzer.sound(NOTE_A4, 5000);
    buzzer.end(5);
  */
}
