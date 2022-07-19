#include <WiFi.h>
#include <PubSubClient.h>

#include "secrets.h"

// wifi
const char* coordinator_ip = "coordinator.local";
const int coordinator_port = 1883;
char* client_name;

void connection_init(PubSubClient &mqtt_client, char* c_name) {
  mqtt_client.setServer(coordinator_ip, coordinator_port);
  mqtt_client.setBufferSize(512);
  client_name = c_name;
}

bool wifi_wakeup() {
  // WIFI SETUP
  Serial.print("[WiFi] Connecting to " + String(wifi_ssid));
  WiFi.begin(wifi_ssid, wifi_password);
  for (int rechecks = 0; rechecks < 10; rechecks++)
  {
    delay(1000);
    Serial.print(".");
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println();
      return true;
    }
  }
  Serial.println();
  return false;
}

bool mqtt_wakeup(PubSubClient &mqtt_client) {
  //MQTT SETUP
  if (mqtt_client.connect(client_name))
    return true;
  else
    return false;
}

int connection_wakeup(PubSubClient &mqtt_client) {

  if (!(WiFi.status() == WL_CONNECTED) && !wifi_wakeup()) {
    Serial.println("WiFi connection failed.");
    return -1;
  }
  
  if (!mqtt_client.connected()) {
    if(!mqtt_wakeup(mqtt_client)) {
      Serial.println("MQTT broker connection failed with error code " + String(mqtt_client.state()));
      return mqtt_client.state();
    } else
      return 200;
  }

  Serial.println("Connection success!");
  
  return 0;
}
