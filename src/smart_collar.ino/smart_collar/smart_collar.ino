#include "connectivity.h"

#include <Adafruit_MPU6050.h>

#include "DHT.h"

#define DHTPIN 19     // Digital pin connected to the DHT sensor

#define PREFIX             "homeassistant"
#define SENSOR_TYPE        "/sensor"
#define BINARY_SENSOR_TYPE "/binary_sensor"
#define DEVICE_UID         "0"
#define DEVICE_NAME        "/smart_collar"

#define SENSOR_PREFIX            PREFIX SENSOR_TYPE DEVICE_NAME
#define BIN_SENSOR_PREFIX        PREFIX BINARY_SENSOR_TYPE DEVICE_NAME

#define TEMPERATURE_CONFIG_TOPIC SENSOR_PREFIX "/temperature/config"
#define HUMIDITY_CONFIG_TOPIC    SENSOR_PREFIX "/humidity/config"
#define HEAT_IDX_CONFIG_TOPIC    SENSOR_PREFIX "/heat_index/config"
#define ACTIVITY_CONFIG_TOPIC    BIN_SENSOR_PREFIX "/activity/config"

#define TEMPERATURE_STATE_TOPIC  SENSOR_PREFIX "/temperature/state"
#define ACTIVITY_STATE_TOPIC     BIN_SENSOR_PREFIX "/activity/state"

#define CONNECTED_SLEEP_TIME      1 * 60 * 1000 * 1000 // microseconds
#define DISCONNECTED_SLEEP_TIME  10 * 60 * 1000 * 1000 // microseconds

const char* temperature_config_payload = "{\"device_class\": \"temperature\", \"unique_id\": \"temperature" DEVICE_UID "\", \"name\": \"Temperature\", \"state_topic\": \"" TEMPERATURE_STATE_TOPIC "\", \"unit_of_measurement\": \"°C\", \"value_template\": \"{{ value_json.temperature}}\", \"expire_after\": 300 }";
const char* humidity_config_payload    = "{\"device_class\": \"temperature\", \"icon\": \"mdi:water\", \"unique_id\": \"humidity" DEVICE_UID "\", \"name\": \"Humidity\", \"state_topic\": \"" TEMPERATURE_STATE_TOPIC "\", \"unit_of_measurement\": \"%\", \"value_template\": \"{{ value_json.humidity}}\", \"expire_after\": 300 }";
const char* heat_index_config_payload  = "{\"device_class\": \"temperature\", \"icon\": \"mdi:heat-wave\", \"unique_id\": \"heatindex" DEVICE_UID "\", \"name\": \"Heat Index\", \"state_topic\": \"" TEMPERATURE_STATE_TOPIC "\", \"unit_of_measurement\": \"°C\", \"value_template\": \"{{ value_json.heat_index}}\", \"expire_after\": 300 }";
const char* activity_config_payload    = "{\"device_class\": \"moving\", \"icon\": \"mdi:cat\", \"unique_id\": \"activity" DEVICE_UID "\", \"name\": \"Pet Activity Status\", \"state_topic\": \"" ACTIVITY_STATE_TOPIC "\", \"expire_after\": 300 }";

double movement_threshold = 0.1;
int measurement_count = 5;

WiFiClient wifi;
PubSubClient mqtt_client(wifi);

void publish_discovery_info() {
  // discovery pub
  mqtt_client.publish((char*) HEAT_IDX_CONFIG_TOPIC, heat_index_config_payload, false);
  mqtt_client.publish((char*) HUMIDITY_CONFIG_TOPIC, humidity_config_payload, false);
  mqtt_client.publish((char*) TEMPERATURE_CONFIG_TOPIC, temperature_config_payload, false);
  mqtt_client.publish((char*) ACTIVITY_CONFIG_TOPIC, activity_config_payload, false);
}

void setup() {
  Serial.begin(115200);
  
  connection_init(mqtt_client, (char*)DEVICE_NAME);
  int rc = connection_wakeup(mqtt_client);

  // connected successfully
  if(rc == 0 || rc == 200)
  {
    publish_discovery_info();

    // setup mpu
    Adafruit_MPU6050 mpu;
    mpu.begin();
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
    delay(2000); // wait a couple of seconds for the accelerometer to start up

    // init mpu delta acceleration loop
    sensors_event_t a, g, temp;
    double prev_a = 0, cur_a = 0, cum_jerk = 0;
    mpu.getEvent(&a, &g, &temp);
    cur_a = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));
    
    // loop and measure
    for (int i = 1; i < measurement_count; i++) {
      mpu.getEvent(&a, &g, &temp);
      prev_a = cur_a;
      cur_a  = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));
      cum_jerk += abs(prev_a - cur_a);
      delay(1000);
    }

    double average_jerk = cum_jerk/(measurement_count - 1);
    Serial.println("Cumulative delta of accelerations: " + String(cum_jerk));
    Serial.println("Average difference of accelerations: " + String(average_jerk));

    // publish activity
    if (average_jerk > movement_threshold)
      mqtt_client.publish(ACTIVITY_STATE_TOPIC, "ON", false);
    else
      mqtt_client.publish(ACTIVITY_STATE_TOPIC, "OFF", false);

    float temperature, humidity, heat_idx;
    int retries = 0;

    // get temperature
    DHT dht(DHTPIN, DHT11);
    dht.begin();
    do {
      temperature = dht.readTemperature();
      humidity    = dht.readHumidity();
      heat_idx    = dht.computeHeatIndex(temperature, humidity, false);
      retries++;
      delay(1000);
    } while((isnan(temperature) || isnan(humidity) || isnan(heat_idx)) && retries < 5); // retry for 5s max
    
    // publish temperature
    String payload_str;
    char payload[256];
    payload_str =  "{ \"temperature\": " + String(temperature);
    payload_str += ", \"humidity\": " + String(humidity);
    payload_str += ", \"heat_index\": " + String(heat_idx);
    payload_str += " }";
    payload_str.toCharArray(payload, payload_str.length() + 1);
    mqtt_client.publish(TEMPERATURE_STATE_TOPIC, payload, false);
    
    // setup wakeup timer
    esp_sleep_enable_timer_wakeup(CONNECTED_SLEEP_TIME);
  } else // disconnected
    esp_sleep_enable_timer_wakeup(DISCONNECTED_SLEEP_TIME);
  
  // go to sleep
  Serial.println("Work finished, going to sleep. Goodnight...");
  Serial.flush();
  delay(500);
  esp_deep_sleep_start();
}

void loop() {} // unused with deep sleep
