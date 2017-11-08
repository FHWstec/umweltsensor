#include <DS3231.h>

// ATmega2560 Spezifikationen:
// Eingangsspannung: 5 V
// Eingangsstromstärke: 40 - 50 mA
// Auflösung: 10 Bit
// Integrale Non-linearität: 1 LSB
// Absolute Genauigkeit: 2 LSB
// Umrechnungszeit: 13 - 260 µs 
// Eingangswiderstand: 100 M Ω
// Eingangswiderstand der Referenzspannung: 32k Ω
//----------------------------------
// Eingangsspannung messen:
long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
//----------------------------------
//#include <DS3231.h>
#include <SD.h>
#include <SPI.h>

File myFile;

DS3231  rtc(SDA, SCL);
int pinCS = 53;
//----------------------------------
int co2 = A0;
//----------------------------------
int gas = A4;

void setup()
{
  // Setup Serial connection
  Serial.begin(9600);
  //----------------------------------
  // Initialisierung des RTC Objects
  rtc.begin();
  // Zeit und Datums Einstellung: 
  //rtc.setDOW(WEDNESDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(13, 34, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(05, 07, 2017);   // Set the date to January 1st, 2014
  //----------------------------------
  // ChipSelector
  pinMode(pinCS, OUTPUT); 
  //---------------------------------- 
  // SD Card Initialization
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed! Insert a volume...");
    return;
  }
  // Initialisierung der Sensorpins
  pinMode(co2, INPUT);
  //----------------------------------
  pinMode(gas, INPUT);
  //----------------------------------
  analogReference(INTERNAL1V1); // 4.9mV Schritte oder bei INTERNAL2V56 
}

void loop()
{ 
  // Wochentag
  Serial.print(rtc.getDOWStr());
  Serial.print(" ");
  //----------------------------------
  // Datum
  Serial.print(rtc.getDateStr());
  Serial.print(" -- ");
  //----------------------------------
  // Zeit 
  Serial.println(rtc.getTimeStr());
  //----------------------------------
  // Temperatur
  Serial.print("Grad Celsius:");
  Serial.print(" -- ");
  Serial.println(int(rtc.getTemp()));
  //----------------------------------
  Serial.print("Gemessene Ausgangsspannung: ");
  Serial.print(" -- ");
  Serial.print(readVcc(), DEC);
  Serial.println("mV");
  //----------------------------------
  //float co2SensorVoltage = analogRead(co2);
  //co2SensorVoltage = co2SensorValue / 1024 * 5.0;
  //----------------------------------
  //PPM = PPM_max * (ADC / 1023);
  //int PPM_max_CO2 = 10000;
  //int PPM_min_CO2 = 350;
  //float PPM_CO2;
  //PPM_CO2 = PPM_max_CO2 * (co2SensorValue / 1023);
  //----------------------------------
  //Serial.print("MG811 CO2 Sensor:");
  //Serial.print(" -- ");
  //Serial.print(co2SensorVoltage);
  //Serial.println("V");
  //Serial.print("MG811 CO2 Sensor:");
  //Serial.print(" -- ");
  //Serial.println(PPM_CO2);
  //----------------------------------
  float gasSensorVoltage;
  //float RS_gas; //  Get the value of RS via in a clear air
  //float R0;  // Get the value of R0 via in H2
  //float ratio; // Get ratio RS_GAS/RS_air
  float gasSensorValue;
  
  for(int x = 0 ; x < 100 ; x++)
    {
        gasSensorValue = gasSensorValue + analogRead(gas);
    }
    gasSensorValue = gasSensorValue/100.0;
  
  gasSensorValue = analogRead(gas);
  gasSensorVoltage = gasSensorValue / 1024 * 5.0;
  /*
  //RS_gas = (5.0-gasSensorVoltage)/gasSensorVoltage; // omit *RL
  //R0 = RS_gas/9.8; // The ratio of RS/R0 is 9.8 in a clear air from Graph (Found using WebPlotDigitizer)
 
  //ratio = RS_gas/R0;
  //----------------------------------
  Serial.print("R0 = ");
  Serial.println(R0);
  Serial.print("RS_ratio = ");
  Serial.println(RS_gas);
  Serial.print("RS/R0 = ");
  Serial.println(ratio);
  */
  //----------------------------------
  int PPM_max_Alcohol = 2000;
  int PPM_min_Alcohol = 100;
  int PPM_max_LPGPropaneButaneH2 = 5000;
  int PPM_min_LPGPropane = 200;
  int PPM_min_ButaneH2 = 300;
  int PPM_max_Methan = 20000;
  int PPM_min_Methan = 5000;
  float PPM_Alcohol;
  float PPM_LPGPropaneButaneH2;
  float PPM_Methan;
  
  PPM_Alcohol = PPM_max_Alcohol * (gasSensorValue / 1023);
  PPM_LPGPropaneButaneH2 = PPM_max_LPGPropaneButaneH2 * (gasSensorValue / 1023);
  PPM_Methan = PPM_max_Methan * (gasSensorValue / 1023);
  
  
  Serial.print("MQ2 Gas Sensor: ");
  Serial.print(" -- ");
  Serial.print(gasSensorVoltage);
  Serial.println("V");
  //----------------------------------
  Serial.print("MQ2 Gas Sensor Alcohol: ");
  Serial.print(" -- ");
  Serial.print(PPM_Alcohol);
  Serial.println("ppm");
  Serial.print("MQ2 Gas Sensor LPG, Propan, Butan, H2: ");
  Serial.print(" -- ");
  Serial.print(PPM_LPGPropaneButaneH2);
  Serial.println("ppm");
  Serial.print("MQ2 Gas Sensor Methan: ");
  Serial.print(" -- ");
  Serial.print(PPM_Methan);
  Serial.println("ppm");
  
  delay(10000);
  
  // Öffne test.txt & schreibe zuvor dargestellte Werte
  myFile = SD.open("umweltsensor.txt", FILE_WRITE);
  if (myFile) {
    // Wochentag
    myFile.print(rtc.getDOWStr());
    myFile.print(" ");
  
    // Datum
    myFile.print(rtc.getDateStr());
    myFile.print(" -- ");
    // Zeit
    myFile.println(rtc.getTimeStr());
    myFile.print("Grad Celsius:"); 
    myFile.print(" -- ");
    myFile.println(int(rtc.getTemp()));
    
    //int co2Sensor = analogRead(co2);
    //----------------------------------
    //int gasSensor = analogRead(gas);
  
    //myFile.print("MG811 CO2 Sensor:");
    //myFile.print(" -- ");
    //myFile.println(co2SensorVoltage);
    //myFile.println("V");
    //myFile.print("MG811 CO2 Sensor:");
    //myFile.print(" -- ");
    //myFile.println(gasSensorValue);
    //----------------------------------
    myFile.print("MQ2 Gas Sensor: ");
    myFile.print(" -- ");
    myFile.println(gasSensorVoltage);
    myFile.println("V");
    //----------------------------------
    myFile.print("MQ2 Gas Sensor Alcohol: ");
    myFile.print(" -- ");
    myFile.print(PPM_Alcohol);
    myFile.println("ppm");
    myFile.print("MQ2 Gas Sensor LPG, Propan, Butan, H2: ");
    myFile.print(" -- ");
    myFile.print(PPM_LPGPropaneButaneH2);
    myFile.println("ppm");
    myFile.print("MQ2 Gas Sensor Methan: ");
    myFile.print(" -- ");
    myFile.print(PPM_Methan);
    myFile.println("ppm");
    myFile.close();
  }
  else { 
    Serial.println("Fatal Error while opening test.txt");
  }
    
  // Ableseintervall
  delay (10000);
}


