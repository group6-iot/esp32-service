#include <PubSubClient.h>

#include <WiFi.h>

#define DHTPIN 15
#define AOUT_PIN 36 // ESP32 pin GIOP36 (ADC0) that connects to AOUT pin of moisture sensor
#define DHTTYPE DHT11
#define RELAY_PIN 16
#define LED_PIN 22


WiFiClient esp_client;
PubSubClient client(esp_client);

void setup() {
    Serial.begin(9600);
    WiFi.begin("NNN", "09052001");
  
//  WiFi.begin("DMTT", "mot23456789");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("[WIFI]: Connecting...");
    }
    Serial.println("[WIFI]: Connected");
    
    client.setServer("broker.emqx.io", 1883);
    client.setCallback(callback);
    client.setKeepAlive(60);

    while (!client.connected()) {
        Serial.println("Connecting to MQTT...");
        if (client.connect("ESP8266Client")) {
            Serial.println("[MQTT]: Connected");
            client.subscribe("device/update-status");
        } else {
            Serial.print("[MQTT]: Failed with state ");
            Serial.println(client.state());
            delay(2000);
        }
    }

    pinMode(RELAY_PIN, OUTPUT);
    pinMode(22, OUTPUT);

}

void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    String str_payload = String((char*)payload);
  
    int equal_idx = str_payload.indexOf("=");
    int splitter_idx = str_payload.indexOf("|");
    String device = str_payload.substring(equal_idx + 1, splitter_idx);

    equal_idx = str_payload.indexOf("=", equal_idx + 1);
    String status = str_payload.substring(equal_idx + 1);

    Serial.println(status);
    Serial.println(device);
    
    if (device=="Pump") {
      if (status == "on") {
          digitalWrite(RELAY_PIN, LOW);
      }
      else if (status == "off") {
          digitalWrite(RELAY_PIN, HIGH);
      }
    }
    
    if (device=="Light") {
      if (status == "on") {
          digitalWrite(LED_PIN, HIGH);
      }
      else if (status == "off") {
          digitalWrite(LED_PIN, LOW);
      }
    }
}

void loop() {
    client.loop();
    int humidity = analogRead(AOUT_PIN)/4; // read the analog value from sensor
    int light = analogRead(34);
    Serial.print("Moisture value: ");
    Serial.println(humidity);
    Serial.print("Light value: ");
    Serial.println(light);
    String payload = "humidity=" + (String)humidity + "|light=" + (String)light;
    client.publish("enviroment/capture", payload.c_str());
    delay(2000);
}
