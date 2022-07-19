#include "connectivity.h"
#include "smart_feeder.h"

#define PREFIX      "homeassistant"
#define SWITCH_TYPE "/switch"
#define BUTTON_TYPE "/button"
#define DEVICE_UID  "0"
#define DEVICE_NAME "/smart_feeder"

#define SWITCH_PREFIX PREFIX SWITCH_TYPE DEVICE_NAME
#define BUTTON_PREFIX PREFIX BUTTON_TYPE DEVICE_NAME

#define PUMP_CONFIG_TOPIC      SWITCH_PREFIX "/pump/config"
#define DISPENSER_CONFIG_TOPIC BUTTON_PREFIX "/dispenser/config"

#define PUMP_COMMAND_TOPIC      SWITCH_PREFIX "/pump/command"
#define PUMP_STATE_TOPIC        SWITCH_PREFIX "/pump/command"
#define DISPENSER_COMMAND_TOPIC BUTTON_PREFIX "/dispenser/command"

const char* pump_config_payload      = "{\"assumed_state\": false, \"ic\": \"mdi:water-pump\", \"optimistic\": true, \"uniq_id\": \"pump" DEVICE_UID "\", \"payload_on\": \"ON\", \"payload_off\": \"OFF\", \"state_on\": \"ON\", \"state_off\": \"OFF\", \"name\": \"Water Pump\", \"stat_t\": \"" PUMP_STATE_TOPIC "\", \"cmd_t\": \"" PUMP_COMMAND_TOPIC "\", \"qos\": 1, \"ret\": true}";
const char* dispenser_config_payload = "{\"unique_id\": \"dispenser" DEVICE_UID "\", \"name\": \"Food Dispenser\", \"command_topic\": \"" DISPENSER_COMMAND_TOPIC "\", \"payload_press\": \"dispense\", \"qos\": 2 }";

// serial
int SERIAL_BAUDRATE = 115200;

// pins and connections
int BOOT_LED      =  2;
int PUMP_PIN      =  5;
int DISPENSER_PIN = 15;

// global variables
static smart_feeder_t feeder;
WiFiClient wifi;
PubSubClient mqtt_client(wifi);

void publish_discovery_info() {
  // pub
  mqtt_client.publish((char*)PUMP_CONFIG_TOPIC, pump_config_payload, false);
  mqtt_client.publish((char*)DISPENSER_CONFIG_TOPIC, dispenser_config_payload, false);
}

void resub_topics() {
  mqtt_client.subscribe((char*) PUMP_COMMAND_TOPIC);
  mqtt_client.subscribe((char*) DISPENSER_COMMAND_TOPIC);
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  delay(1000);

  // setup
  pinMode(BOOT_LED, OUTPUT);
  connection_init(mqtt_client, (char*) DEVICE_NAME);
  mqtt_client.setCallback(mqtt_callback);
  feeder_init(feeder, PUMP_PIN, DISPENSER_PIN);
  close_pump(feeder);
  
  // SHOW BLINKING LED AT SUCCESSFUL BOOT
  for (int i = 0; i < 10; i++)
  {
    digitalWrite(BOOT_LED, HIGH);
    delay(100);
    digitalWrite(BOOT_LED, LOW);
    delay(100);
  }
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
  if (String(topic) == String(PUMP_COMMAND_TOPIC)) {
    
    if (str_message == "ON") {
      Serial.println("pump ON");
      open_pump(feeder);
    } else if (str_message == "OFF") {
      Serial.println("pump OFF");
      close_pump(feeder);
    } else
      Serial.println("Unrecognized pump command.");
  } else if (String(topic) == String(DISPENSER_COMMAND_TOPIC)) {
    if (str_message == "dispense") {
      dispense_food(feeder);
    }
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
