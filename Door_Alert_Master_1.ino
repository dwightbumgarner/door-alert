#include <ESP8266WiFi.h>
#include <espnow.h>

#define RETRY_INTERVAL 5000
#define SEND_INTERVAL 1000
#define SWITCH_PIN D

// the following three settings must match the slave settings
uint8_t remoteMac[] = {0x82, 0x88, 0x88, 0x88, 0x88, 0x88};
const uint8_t channel = 14;
struct __attribute__((packed)) DataStruct {
  boolean isDoorOpen;
};

DataStruct statusData;

unsigned long sentStartTime;
unsigned long lastSentTime;

void sendData() {
  uint8_t bs[sizeof(statusData)];
  memcpy(bs, &statusData, sizeof(statusData));

  sentStartTime = micros();
  esp_now_send(NULL, bs, sizeof(statusData)); // NULL means send to all peers
}

void sendCallBackFunction(uint8_t* mac, uint8_t sendStatus) {
  unsigned long sentEndTime = micros();
  Serial.printf("Send To: %02x:%02x:%02x:%02x:%02x:%02x ", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("Door Status: %0d ", statusData.isDoorOpen);
  Serial.printf("Trip micros: %4lu, ", sentEndTime - sentStartTime);
  Serial.printf("Status: %s\n", (sendStatus == 0 ? "Success" : "Failed"));
}

void setup() {
  WiFi.mode(WIFI_STA); // Station mode for esp-now controller
  WiFi.disconnect();

  Serial.begin(115200);
  Serial.println();
  Serial.println("ESP-Now Transmitter");
  Serial.printf("Transmitter mac: %s \n", WiFi.macAddress().c_str());
  Serial.printf("Receiver mac: %02x:%02x:%02x:%02x:%02x:%02x\n", remoteMac[0], remoteMac[1], remoteMac[2], remoteMac[3], remoteMac[4], remoteMac[5]);
  Serial.printf("WiFi Channel: %i\n", channel);

  if (esp_now_init() != 0) {
    Serial.println("ESP_Now init failed...");
    delay(RETRY_INTERVAL);
    ESP.restart();
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(remoteMac, ESP_NOW_ROLE_SLAVE, channel, NULL, 0);
  esp_now_register_send_cb(sendCallBackFunction);

  statusData.isDoorOpen = true;
  sendData();
  //ESP.deepSleep(0);
}

void loop() {
 sendData();
 delay(100);
}
