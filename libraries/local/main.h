#ifndef MAIN_H

void setup();
void loop();
void makeMoistureMeasurement();
void connectWifi(char* ssid, char* wifiPassword);
PubSubClient configureMQTTClient();
void connectMQTT();
void publishMeasurements();
void callback(char* topic, byte* payload, unsigned int length);
void initializeTopics();
#endif