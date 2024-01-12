#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Replace with your network credentials
const char* ssid = "Smart Wifi";
const char* password = "altiuxwifi@2020";

// Replace with your MQTT broker IP address
const char* mqtt_server = "broker.emqx.io";

// Replace with your MQTT topics
const char* subscribe_topic = "mqtt_box/commands";
const char* publish_topic = "mqtt_box/readings";

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_BME280 bme;
const int pirPin = 4;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  pinMode(pirPin, INPUT);
  Serial.println("Ready!");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }

  Serial.print("Received message: ");
  Serial.println(msg);

  if (msg == "TEMP") {
    float temp = bme.readTemperature();
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" *C");
    String TEMP = "Temparature " + String(temp);
    client.publish(publish_topic, TEMP.c_str());
  } else if (msg == "HUMI") {
    float humidity = bme.readHumidity();
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    String HUMI = "Humidity " + String(humidity);
    client.publish(publish_topic, HUMI.c_str());
  } else if (msg == "Motion") {

    int pirValue = digitalRead(pirPin);

    if (pirValue == HIGH) {
      Serial.print("Motion Detected \n");
      String motion = "Motion Detected";
      client.publish(publish_topic, motion.c_str());

    } else if (pirValue == LOW) {
      Serial.print("Motion Not Detected \n");
      String Nmotion = "Motion Not Detected";
      client.publish(publish_topic, Nmotion.c_str());
    }

  }
  else {
    String I = "Invalid Command!";
    Serial.println("Invalid Command");
    client.publish(publish_topic, I.c_str());
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect("esp32")) {
      Serial.println("Connected to MQTT broker!");
      client.subscribe(subscribe_topic);
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}
