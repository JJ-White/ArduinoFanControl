#include "PinChangeInterrupt.h"

/* ARGB variables */
const int argb_pins[] = { 10, 16, 17 };

/* FAN variables */
const int fan_nr = 5;
const int fan_pwm_pins[] = { 3, 5, 6, 9, 11 };
const int fan_rpm_pins[] = { 2, 4, 7, 8, 12 };
const void* fan_isr[] = { &fan0_isr, &fan1_isr, &fan2_isr, &fan3_isr, &fan4_isr};
int fan_int_count[fan_nr] = {0};
unsigned long fan_millis[fan_nr] = {0};
int fan_target[fan_nr];

/* NTC variables
   Info for reading NTC and converting to temperature:
   https://www.digikey.com/en/maker/projects/how-to-measure-temperature-with-an-ntc-thermistor/4a4b326095f144029df7f2eca589ca54
   For calculating NTC Beta from calibration readings:
   https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
*/
const int ntc_pins[] = { A0, A6, A7 };
const int ntc_r1 = 6800;
const float ntc_beta = 3306.17;
const float ntc_cal_temp = 39.3 + 273.15;
const float ntc_cal_res = 6000.0;

/* NTC methods */
float adc_to_temp(int ntc_pin) {
  int r2 = ntc_r1 * (1023 / (1023 - (float)analogRead(ntc_pin)) - 1);
  if ( r2 == 0 ) return 0.0;
  else return (ntc_beta * ntc_cal_temp) / (ntc_beta + (ntc_cal_temp * log(r2 / ntc_cal_res))) - 273.15;
}

void print_temps() {
  Serial.print("NTC0: ");
  Serial.print(adc_to_temp(ntc_pins[0]));
  Serial.print(" NTC1: ");
  Serial.print(adc_to_temp(ntc_pins[1]));
  Serial.print(" NTC2: ");
  Serial.print(adc_to_temp(ntc_pins[2]));
  Serial.println("");
}

/* FAN methods */
void init_fans() {
  for ( int i = 0; i < fan_nr; i++) {
    pinMode(fan_pwm_pins[i], OUTPUT);
    pinMode(fan_rpm_pins[i], INPUT_PULLUP);
    attachPCINT(digitalPinToPCINT(fan_rpm_pins[i]), fan_isr[i] , FALLING);
    fan_target[i] = 255;
  }
  set_fans_to_target();
}

void set_fans_to_target() {
  for ( int i = 0; i < fan_nr; i++) {
    analogWrite(fan_pwm_pins[i], fan_target[i]);
  }
}

int get_fan_rpm(int fan) {
  if ( fan_int_count[fan] == 0) return 0;
  int rpm = (fan_int_count[fan] / 2) * (60000 / (float)(millis() - fan_millis[fan]));
  fan_int_count[fan] = 0;
  fan_millis[fan] = millis();
  return rpm;
}

void print_fan_rpms() {
  for ( int i = 0; i < fan_nr; i++) {
    Serial.print("F");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(get_fan_rpm(i));
    Serial.print(" ");
  }
  Serial.println("");
}

void fan0_isr() {
  (fan_int_count[0])++;
}
void fan1_isr() {
  (fan_int_count[1])++;
}
void fan2_isr() {
  (fan_int_count[2])++;
}
void fan3_isr() {
  (fan_int_count[3])++;
}
void fan4_isr() {
  (fan_int_count[4])++;
}

/* Program */
void setup() {
  // Serial setup
  Serial.begin(115200);
  Serial.println("Setup...");

  init_fans();

  Serial.println("Starting loop");
}

void loop() {
  print_temps();
  print_fan_rpms();
  Serial.println();
  delay(1000);
}
