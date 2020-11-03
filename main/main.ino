/**
   This file is part of De-Druppel/DruppelController
   For license information, please view the LICENSE file that was distributed with this source code.
*/

#include <Arduino.h>
#include <PubSubClient.h>
#include <credentials.h>
#include <ESP8266WiFi.h>
#include <main.h>

WiFiClient wifiClient;
PubSubClient pubsubClient;
String ESP_ID = String(ESP.getChipId());
const char* MOISTURE_TOPIC;
const char* TRESHOLD_TOPIC;
const char* SUBSCRIPTION_TOPIC;
const int MOISTURE_SENSOR_PIN = A0;
float moisturePercentage;
int wifiStatus = WL_IDLE_STATUS;

//All variables used for the delays
long previousMillis = 0;
long publishMoistureInterval = 1000 * 60 * 5;

/// Setup is called once when the ESP8266 is starting.
/// Used for configuration.
void setup() {
  Serial.begin(115200);
  initializeTopics();
  pubsubClient = configureMQTTClient();

  delay(1000);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  Serial.printf("Attempting to connect to Wifi SSID: %s. \n", SSID);
  wifiStatus = WiFi.begin(SSID, WIFI_PASSWORD);
  
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); 
}

/*
  get the currrent mositure percentage by making a measurement with the sensor
*/
void makeMoistureMeasurement() {
  //read analog signal and create a mositure percentage
  float sensor0 = analogRead(MOISTURE_SENSOR_PIN);
  //set the current percentage
  moisturePercentage = 100.00 - ( ( 100.00 * sensor0 ) / 1023.00 );
  Serial.printf("Measured moisture at %.2f. \n", moisturePercentage);
}

/// Loop is called every cycle of the ESP8266.
void loop() {
  //We don't need a delay on making a measurement right?
  makeMoistureMeasurement();

    //connect to the mqtt server only when wifi is connected and there is no mqtt connection
    if (!pubsubClient.connected()) {
      connectMQTT();
    } else {
      //Add a delay on publishing the measurement
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis > publishMoistureInterval) {
        // save the last time you entered this delay
        previousMillis = currentMillis;
        //publish the readings
        publishMeasurements();
      }
    }

  delay(1000);
}

/// Configure the MQTT client.
/// @pubSubClient client that is configured.
/// @wifiClient wifi client that is passed to the pubSubClient.
/// @mqttHost Hostname or ip of the MQTT server you want to connect to.
/// @mqttPort Port that is used to connect to the MQTT server.
/// returns: Configured PubSubClient object.
PubSubClient configureMQTTClient() {
  PubSubClient pubsubClient(wifiClient);
  pubsubClient.setServer(MQTT_HOST, MQTT_PORT);
  return pubsubClient;
}

/// Connect to the MQTT client.
/// pubSubClient needs the be configured prior to running this function.
/// @mqttId Unique ID that is used for connecting to the MQTT server.
/// @mqttUser Username for the MQTT server
/// @mqttPassword Password for the MQTT server.
void connectMQTT() {
  Serial.println("Connecting to MQTT broker");
  pubsubClient.connect(ESP_ID.c_str(), MQTT_USER, MQTT_PASSWORD);
  if (pubsubClient.connected()) {
    Serial.println("Successfully connected to MQTT broker.");
    pubsubClient.subscribe(SUBSCRIPTION_TOPIC);
  }
  else{
    Serial.println("Failed to connect");
  }

  pubsubClient.setCallback(callback);
}


//method used for publishing last measurements to mqtt broker
void publishMeasurements() {
  //publish last moisture percentage
  pubsubClient.publish(MOISTURE_TOPIC, String(moisturePercentage).c_str());
}

/**
   The callback is called whenever a message is recieved on any subscribed topic.
   Depending on the specific topic, it will either forcefully water the plant
   or overwrite the moistureThreshold at which it will be automatically watered.
*/
void callback(char* topic, byte* payload, unsigned int length) {
  if (topic == TRESHOLD_TOPIC) {
    // Overwrite the threshold.
  }
}

/**
   Use strings to set the topics for the pubSubClient and copies them to character arrays so the String objects
   Don't have to be kept in memory.
*/
void initializeTopics() {
  String BASE_TOPIC = String("Garden/" + ESP.getChipId());
  MOISTURE_TOPIC = String(BASE_TOPIC + "/Moisture").c_str();
  TRESHOLD_TOPIC = String(BASE_TOPIC + "/Config/Treshold").c_str();
  SUBSCRIPTION_TOPIC = String(BASE_TOPIC + "/*").c_str();
}
