
/*
 * Info for reading NTC and converting to temperature:
 * https://www.digikey.com/en/maker/projects/how-to-measure-temperature-with-an-ntc-thermistor/4a4b326095f144029df7f2eca589ca54
 * For calculating NTC Beta from calibration readings:
 * https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
 */

const int ntc_pin = A7;
const int ntc_r1 = 6800;
const float ntc_beta = 3306.17;
const float ntc_cal_temp = 39.3 + 273.15;
const float ntc_cal_res = 6000.0;

int adc_to_temp(int adc_val){
  Serial.print("adc: ");
  Serial.print(adc_val);
 
  float volt = (float)map(adc_val, 0, 1023, 0, 5000) / 1000.0;
  Serial.print(" volt: ");
  Serial.print(volt);
  
  float r2 = ((float)volt * (float)ntc_r1) / (5.0 - (float)volt);
  Serial.print(" r2: ");
  Serial.print(r2);
  
  float temp = (ntc_beta * ntc_cal_temp) / (ntc_beta + (ntc_cal_temp * log(r2 / ntc_cal_res)));
  temp -= 273.15; 
  Serial.print(" temp: ");
  Serial.print(temp);
  Serial.println("");
  return temp;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup");
}

void loop() {
  int raw = analogRead(ntc_pin);
  adc_to_temp(raw);
  delay(500);
}
