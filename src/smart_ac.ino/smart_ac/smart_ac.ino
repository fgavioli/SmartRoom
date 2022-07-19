#include "connectivity.h"

#define PREFIX        "homeassistant"
#define SWITCH_TYPE   "/switch"
#define DEVICE_UID    "0"
#define DEVICE_NAME   "/smart_ac"
#define SWITCH_PREFIX PREFIX SWITCH_TYPE DEVICE_NAME

#define AC_CONFIG_TOPIC  SWITCH_PREFIX "/ac/config"
#define AC_COMMAND_TOPIC SWITCH_PREFIX "/ac/command"
#define AC_STATE_TOPIC   SWITCH_PREFIX "/ac/command"

const char* ac_config_payload = "{\"assumed_state\": false, \"optimistic\": \"true\", \"ic\": \"mdi:air-conditioner\", \"uniq_id\": \"ac" DEVICE_UID "\", \"payload_on\": \"ON\", \"payload_off\": \"OFF\", \"stat_on\": \"ON\", \"stat_off\": \"OFF\", \"name\": \"Air conditioning\", \"stat_t\": \"" AC_STATE_TOPIC "\", \"cmd_t\": \"" AC_COMMAND_TOPIC "\", \"qos\": 1, \"ret\": true}";

// serial
int SERIAL_BAUDRATE = 115200;

// pins and connections
int AC_LED_PIN = 15;

// global variables
WiFiClient wifi;
PubSubClient mqtt_client(wifi);

void publish_discovery_info() {
  mqtt_client.publish((char*)AC_CONFIG_TOPIC, ac_config_payload, false);
}

void resub_topics() {
  mqtt_client.subscribe((char*) AC_COMMAND_TOPIC);
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  delay(1000);

  // setup
  pinMode(AC_LED_PIN, OUTPUT);
  digitalWrite(AC_LED_PIN, LOW);
  connection_init(mqtt_client, (char*) DEVICE_NAME);
  mqtt_client.setCallback(mqtt_callback);
}

void mqtt_callback(char* topic, byte* message, unsigned int length)
{
  char msg[256];
  int i;
  for (i = 0; i < length; i++)
    msg[i] = (char) message[i];
  msg[i] = '\0';
  String str_message = String(msg);
  Serial.println("New message on " + String(topic) + "! Contents: " + str_message);
  if (String(topic) == String(AC_COMMAND_TOPIC)) {
    if (str_message == "ON") {
      Serial.println("AC ON");
      digitalWrite(AC_LED_PIN, HIGH);
    } else if (str_message == "OFF") {
      Serial.println("AC OFF");
      digitalWrite(AC_LED_PIN, LOW);
    } else
      Serial.println("Unrecognized AC command.");
  } else
      Serial.println("Unrecognized topic " + String(topic));
}

void loop() {
  int rc = connection_wakeup(mqtt_client);
  if (rc == 0 || rc == 200)
  {
    if (rc == 200) {
      publish_discovery_info();
      resub_topics();
    }
    // spin
    mqtt_client.loop();
    delay(100);
  } else {
    Serial.println("Wifi or MQTT broker unavailable, retrying in 30s...");
    delay(30 * 1000);
    return;
  } 
}
