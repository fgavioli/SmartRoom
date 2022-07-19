/*
 * This file implements the control logic of a small pump
 * Tested with a 5V pump powered through a relay.
 */

typedef struct pump {
  unsigned int ctl_pin;
  bool active;
  
} pump_t;

void init_pump(pump_t &pump, int pin_id)
{
  pump.ctl_pin = pin_id;
  pump.active = false;
  
  pinMode(pin_id, OUTPUT);
  digitalWrite(pump.ctl_pin, HIGH);
}

void pump_on(pump_t &pump)
{
  pump.active = true;
  digitalWrite(pump.ctl_pin, LOW);
}

void pump_off(pump_t &pump)
{
  pump.active = false;
  digitalWrite(pump.ctl_pin, HIGH);
}
