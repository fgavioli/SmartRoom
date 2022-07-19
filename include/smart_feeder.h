#include "pump.h"
#include "dispenser.h"

// things
typedef struct smart_feeder {
  pump_t water_pump;
  dispenser_t dispenser;
} smart_feeder_t;

void feeder_init(smart_feeder_t &feeder, int pump_pin, int dispenser_pin) {
  init_pump(feeder.water_pump, pump_pin);
  init_dispenser(feeder.dispenser, dispenser_pin);
}

void get_sensor_data(smart_feeder_t &feeder)
{
  Serial.println("Publishing sensor data into the MQTT network");
}

void open_pump(smart_feeder_t &feeder)
{
  pump_on(feeder.water_pump);
}

void close_pump(smart_feeder_t &feeder)
{
  pump_off(feeder.water_pump);
}

void dispense_food(smart_feeder_t &feeder)
{
  dispense(feeder.dispenser);  
}
