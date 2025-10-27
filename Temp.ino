#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT22

const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* MQTT_BROKER_IP = "192.168.1.100";
const int   MQTT_PORT      = 1883;
const char* MQTT_TOPIC     = "home/temperature";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
unsigned long lastSend = 0;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
}

void connectMQTT() {
  while (!client.connected()) {
    if (client.connect("esp32-temp-publisher")) break;
    delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  connectWiFi();
  client.setServer(MQTT_BROKER_IP, MQTT_PORT);
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!client.connected()) connectMQTT();
  client.loop();

  unsigned long now = millis();
  if (now - lastSend >= 2000) {
    lastSend = now;

    float t = dht.readTemperature(); // Celsius
    if (!isnan(t)) {
      char payload[16];
      dtostrf(t, 0, 2, payload); // e.g. "24.57"
      client.publish(MQTT_TOPIC, payload, false);
      Serial.print("Published temp: ");
      Serial.println(payload);
    } else {
      Serial.println("DHT read failed");
    }
  }
}
