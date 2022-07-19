
/*
 * This file implements the control logic of a 5V servo
 * controlling a food dispenser.
 * Uses timers 0, 1, 2, 3 of the ESP32.
 */

#include <ESP32Servo.h>

enum servo_angle_t {LEFT = 20, RIGHT = 160};

typedef struct dispenser {
  servo_angle_t angle;
  Servo servo;
} dispenser_t;

void init_dispenser(dispenser_t &dispenser, int pin_id)
{
  pinMode(pin_id, OUTPUT);
  dispenser.angle = LEFT;
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  dispenser.servo.setPeriodHertz(50);
  dispenser.servo.attach(pin_id, 1000, 2000);
  dispenser.servo.write(dispenser.angle);
}

void dispense(dispenser_t &dispenser)
{
  if(dispenser.angle == LEFT)
    dispenser.angle = RIGHT;
  else
    dispenser.angle = LEFT;
  Serial.println("Dispensing...");
  dispenser.servo.write(dispenser.angle);
}
