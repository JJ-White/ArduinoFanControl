#include <avr/wdt.h>
#include "PinChangeInterrupt.h"
#include <FastLED.h>

/* Control variables */
const int target_liquid_temp = 40;

/* ARGB variables */
#define nr_leds 96
CRGB argb0[nr_leds];
CRGB argb1[nr_leds];
CRGB argb2[nr_leds];

/* FAN variables */
const int fan_nr = 5;
const int fan_pwm_pins[] = { 2, 5, 6, 9, 11 };
const int fan_rpm_pins[] = { 3, 4, 7, 8, 12 };
const void* fan_isr[] = { &fan0_isr, &fan1_isr, &fan2_isr, &fan3_isr, &fan4_isr};
int fan_int_count[fan_nr] = {0};
unsigned long fan_millis[fan_nr] = {0};
int fan_target[fan_nr];
#define PUMP 4
#define MIN_FAN 20

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
#define WATER 2

/* Case input variables */
const int case_fan_pin = A1;
const int case_led_pin = 13;
enum fanspeed { none, low, medium, high };

/* Control methods */
void temp_control() {
  float temp = adc_to_temp(ntc_pins[WATER]);
  enum fanspeed fan_setting = get_case_fan();
  if ( fan_setting == fanspeed::high || fan_setting == fanspeed::none )
    for ( int i = 0; i < fan_nr; i++)
      fan_target[i] = 100;
  else {
    int target = pow(1.20, temp - 20);
    if ( target < 0 ) target = 0;
    else if (target > 100) target = 100;
    for ( int i = 0; i < fan_nr; i++)
      fan_target[i] = target;
  }
  //  else if ( fan_setting == fanspeed::medium )
  //    for ( int i = 0; i < fan_nr; i++)
  //      fan_target[i] = 50;
  //  else if ( fan_setting == fanspeed::low )
  //    for ( int i = 0; i < fan_nr; i++)
  //      fan_target[i] = 25;
  set_fans_to_target();
}

/* ARGB methods */
void init_argb() {
  FastLED.addLeds<WS2812B, 10, GRB>(argb0, nr_leds);
  FastLED.addLeds<WS2812B, 16, GRB>(argb1, nr_leds);
  FastLED.addLeds<WS2812B, 17, GRB>(argb2, nr_leds);
  LEDS.setBrightness(255);

  for ( int i = 0; i < nr_leds; i++) { // Top
    argb1[i] = CRGB::Red;
    argb1[i].fadeLightBy(0);
  }
  for ( int i = 0; i < nr_leds; i++) { // GPU
    argb2[i] = CRGB::Red;
    argb2[i].fadeLightBy(0);
  }
  for ( int i = 0; i < nr_leds; i++) { // Bottom
    argb0[i] = CRGB::Red;
    argb0[i].fadeLightBy(0);
  }

  FastLED.show();
}

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
    fan_target[i] = 100;
  }
  set_fans_to_target();
}

void set_fans_to_target() {
  for ( int i = 0; i < fan_nr; i++) {
    if ( i == PUMP ) {
      int target = fan_target[i] * 2;
      if ( target < 0 ) target = 0;
      else if (target > 100) target = 100;
      analogWrite(fan_pwm_pins[i], map(target, 0, 100, 0, 255));
      Serial.print("Pumpspeed: ");
      Serial.println(target);
    }
    else {
      int target = fan_target[i];
      if ( target < MIN_FAN )
        target = MIN_FAN;
      target += random(2); // Add random jitter to prevent harmonics
      analogWrite(fan_pwm_pins[i], map(target, 0, 100, 0, 255));
      Serial.print("Fanspeed: ");
      Serial.println(target);
    }
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

/* Case input methods */
int get_case_fan() {
  int raw = analogRead(case_fan_pin);
  if ( raw < 600 ) return fanspeed::low;
  else if ( raw >= 600 && raw < 900) return fanspeed::medium;
  else if ( raw > 900 ) return fanspeed::high;
  else return fanspeed::none;
}

void print_case_input() {
  Serial.print("CFAN: ");
  Serial.print(get_case_fan());
  Serial.print(" CLED: ");
  Serial.println(digitalRead(case_led_pin));
}

void set_leds() {
  if (digitalRead(case_led_pin))
    LEDS.setBrightness(255);
  else
    LEDS.setBrightness(0);
  FastLED.show();
}

/* Program */
void setup() {
  // Setup watchdog
  //wdt_enable(WDTO_4S); // Disable watchdog because bootloader doesn't reset it.

  // Serial setup
  Serial.begin(115200);
  Serial.println("Setup...");

  // Setup fans
  init_fans();

  // Setup ARGB
  init_argb();

  //Setup case
  pinMode(case_led_pin, INPUT);

  Serial.println("Starting loop");
}

void loop() {
  //wdt_reset(); // Disable watchdog because bootloader doesn't reset it.
  Serial.print("[");
  Serial.print(millis() / 1000);
  Serial.println("]");

  print_temps();
  print_fan_rpms();
  print_case_input();
  set_leds();
  temp_control();

  Serial.println();
  delay(1000);
}
