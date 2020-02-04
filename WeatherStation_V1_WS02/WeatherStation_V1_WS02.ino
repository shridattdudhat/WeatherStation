
#include <WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7
#include "Adafruit_Si7021.h"

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_Si7021 sensor = Adafruit_Si7021();

//-------- Customise these values -----------
const char* ssid = "Internet";
const char* password = "00000000";

#define ORG "v1rzzy"
#define DEVICE_TYPE "UrbanNaps"
#define DEVICE_ID "UNP01"
#define TOKEN "S0Uq!jdo4kUiHCeXqx"
//-------- Customise the above values --------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char eventTopic[] = "temperature";
const char cmdTopic[] = "iot-2/cmd/led/fmt/json";



WiFiClient wifiClient;

void callback(char* topic, byte* payload, unsigned int payloadLength) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < payloadLength; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if (payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
PubSubClient client(server, 1883, callback, wifiClient);

int publishInterval = 10000;
long lastPublishMillis;

void setup() {
  Serial.begin(9600); Serial.println();
  client.connect(clientId, authMethod, token);
  sensor.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  wifiConnect();
  mqttConnect();
}

void loop() {
  if (millis() - lastPublishMillis > publishInterval) {
    publishData();
    lastPublishMillis = millis();
  }

  if (!client.loop()) {
    mqttConnect();
  }
}

void wifiConnect() {
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("nWiFi connected, IP address: "); Serial.println(WiFi.localIP());

}

void mqttConnect() {

  if (!!!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!!!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    if (client.subscribe(cmdTopic)) {
      Serial.println("subscribe to responses OK");
    } else {
      Serial.println("subscribe to responses FAILED");
    }
    Serial.println();
  }
}


void publishData() {
  // read the input on analog pin 0:

  float temp = 67;
  float Hum = 76;

  //  float temp = sensor.readTemperature();
  //  float Hum = sensor.readHumidity();
  float hPa = 0;
  float alt =  0;


  String payload = "{\"d\":{\"temp\":";
  payload += String(temp);
  payload += ",\"hum\":";
  payload += String(Hum);
  payload += ",\"hPa\":";
  payload += String(hPa);
  payload += ",\"alt\":";
  payload += String(alt);
  payload += "}}";

  Serial.print("Sending payload: "); Serial.println(payload);

  if (client.publish(eventTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
}
