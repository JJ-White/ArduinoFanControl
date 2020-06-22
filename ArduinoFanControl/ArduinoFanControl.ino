
/*
   Info for reading NTC and converting to temperature:
   https://www.digikey.com/en/maker/projects/how-to-measure-temperature-with-an-ntc-thermistor/4a4b326095f144029df7f2eca589ca54
   For calculating NTC Beta from calibration readings:
   https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
*/


// NTC constants
const int ntc_pins[] = { A0, A6, A7 };
const int ntc_r1 = 6800;
const float ntc_beta = 3306.17;
const float ntc_cal_temp = 39.3 + 273.15;
const float ntc_cal_res = 6000.0;

float adc_to_temp(int ntc_pin) {
  int r2 = ntc_r1 * (1023 / (1023 - (float)analogRead(ntc_pin)) - 1);
  if ( r2 == 0 ) return 0.0;
  else return (ntc_beta * ntc_cal_temp) / (ntc_beta + (ntc_cal_temp * log(r2 / ntc_cal_res))) - 273.15;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup");
}

void loop() {
  Serial.print("NTC0: ");
  Serial.print(adc_to_temp(ntc_pins[0]));
  Serial.print(" NTC1: ");
  Serial.print(adc_to_temp(ntc_pins[1]));
  Serial.print(" NTC2: ");
  Serial.print(adc_to_temp(ntc_pins[2]));
  Serial.println("");

  delay(500);
}
