/**
* Setup time for 12 day
                Morning
                Noon
                Evening
  Lcd 16x2
          Display time
  DS3231
          Reading time
  SIM800l
          Send sms to phone number to notification
  Buzz
          Ring
*/
/**
*------------------------------------------------------------------------------------------------
* Library
*------------------------------------------------------------------------------------------------
*/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID     "TMPL6wjI0FaSd"
#define BLYNK_TEMPLATE_NAME   "Smart Cabinet"
#define BLYNK_AUTH_TOKEN      "suvsWSf4Z7JclZT4zDCF0EELA3oYENl_"

#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "time.h"
#include <EEPROM.h>

/**
*------------------------------------------------------------------------------------------------
* Pin define
*------------------------------------------------------------------------------------------------
*/
#define M_step        25
#define M_dir         4
#define N_dir         33
#define N_step        32
#define E_dir         26
#define E_step        27
#define M_calib       13
#define N_calib       14
#define E_calib       19
#define TX_sim        17
#define RX_sim        16
#define Buzz          15
// #define Buzz          16
#define M_button      12
#define Check_Thuoc    5
#define E_button      18
#define Root_button   30
#define MORNING       1
#define NOON          2
#define EVENING       3

#define SimSerial Serial2

#define EEPROM_SIZE    256
#define M_hour_addr    1
#define M_min_addr     2
#define N_hour_addr    3
#define N_min_addr     4
#define E_hour_addr    5
#define E_min_addr     6
#define DayFirst_addr  7

/*
*------------------------------------------------------------------------------------------------
* Global variable
*------------------------------------------------------------------------------------------------
*/
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 3600;
// const char* ssid     = "FPT-Tang 4";
// const char* password = "0359653222";
// const char* ssid     = "Smart_cabinet";
// const char* password = "12345678";
const char* ssid     = "TP-LINK_436132";
const char* password = "123456789";

typedef struct setupTime {
  unsigned char M_hour;
  unsigned char M_minute;
  unsigned char N_hour;
  unsigned char N_minute;
  unsigned char E_hour;
  unsigned char E_minute;
} setupTime;

typedef struct statusWorking{
  unsigned char M_status;
  unsigned char N_status;
  unsigned char E_status;
  unsigned char day_status;
} statusWorking;

typedef struct thisTime {
  char _hour[3];
  char _minute[3];
} thisTime;

statusWorking statusOfDay;
setupTime timeSetup;
thisTime currentTime;

unsigned char counter = 0;
int check_run = 0;
/*
*------------------------------------------------------------------------------------------------
* Function
*------------------------------------------------------------------------------------------------
*/
void Initial_Pin(){
  pinMode(M_step, OUTPUT);
  pinMode(M_dir, OUTPUT);
  pinMode(N_step, OUTPUT);
  pinMode(N_dir, OUTPUT);
  pinMode(E_step, OUTPUT);
  pinMode(E_dir, OUTPUT);
  pinMode(M_button, INPUT_PULLUP);
  pinMode(Check_Thuoc, INPUT);
  pinMode(E_button, INPUT);
  pinMode(M_calib, INPUT);
  pinMode(N_calib, INPUT_PULLUP);
  pinMode(E_calib, INPUT);
  pinMode(Root_button, INPUT);
  pinMode(Buzz, OUTPUT);
}

void testPin(){
  Serial.print("M_button: ");
  Serial.print(digitalRead(M_button));
  Serial.print(" ");
  Serial.print("Check_Thuoc: ");
  Serial.print(digitalRead(Check_Thuoc));
  Serial.print(" ");
  Serial.print("E_button: ");
  Serial.print(digitalRead(E_button));
  Serial.print(" ");
  Serial.print("M_calib: ");
  Serial.print(digitalRead(M_calib));
  Serial.print(" ");
  Serial.print("N_calib: ");
  Serial.print(digitalRead(N_calib));
  Serial.print(" ");
  Serial.print("E_calib: ");
  Serial.println(digitalRead(E_calib));
  
}

/**
 * Initial data
*/

void Initial_Data(){
  timeSetup.E_hour = 0;
  timeSetup.E_minute = 0;
  timeSetup.M_hour = 0;
  timeSetup.M_minute = 0;
  timeSetup.N_hour = 0;
  timeSetup.N_minute = 0;
  statusOfDay.M_status = 0;
  statusOfDay.N_status = 0;
  statusOfDay.E_status = 0;
}

/*
* Roll motor to beginning 
*/
void Motor_goToNextDay(unsigned char dirPin, unsigned char stepPin, unsigned char placePin){
  // digitalWrite(dirPin, HIGH);
  while(1){
      for(unsigned char i = 0; i < 5; i++){
        Motor_stepOne(stepPin);
      }
      if(digitalRead(placePin) == 0){
        break;
      }
    check_run++;
  }
}
void Motor_stepOne(unsigned char stepPin) {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(5000);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(5000);
}

void Motor_goToInit(unsigned char dirPin, unsigned char stepPin, unsigned char calibPin){
  while(digitalRead(calibPin) == 1){
    Motor_stepOne(stepPin);
    if(digitalRead(calibPin) == 0){
      break;
    }
  }
}

void notification(unsigned char mode)
{
  unsigned char i = 0;
  if (mode == 1)
  {
    digitalWrite(Buzz, HIGH);
    delay(100);
    digitalWrite(Buzz, LOW);
  }
  if (mode == 2)
  {
    for (i = 0; i < 2; i++)
    {
      digitalWrite(Buzz, HIGH);
      delay(200);
      digitalWrite(Buzz, LOW);
      delay(200);
    }
  }
  if (mode == 3)
  {
    for (i = 0; i < 6; i++)
    {
      digitalWrite(Buzz, HIGH);
      delay(100);
      digitalWrite(Buzz, LOW);
      delay(100);
    }
  }
}

void Initial_Wifi(){
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println("");
  Serial.println("WiFi connected.");
}

/**
 * Returns the motor to its default state
*/
void Initial_Motor(){
  digitalWrite(E_dir, HIGH);
  digitalWrite(N_dir, HIGH);
  digitalWrite(M_dir, HIGH);
  Motor_goToInit(E_dir,E_step,E_calib);
}

/**
 * Check medicine still exist
 * Check sau 5p ma thuoc van con thi coi keu
*/
void CheckStillMedicine(){
  if(statusOfDay.M_status == 1){
    if(CompareSetupTime(timeSetup.M_hour, timeSetup.M_minute+5)){
      if((digitalRead(Check_Thuoc) == 0) && (counter < 3)){
        notification(2);
        counter++;
      } else if((digitalRead(Check_Thuoc) == 0) && (counter == 2)){
        sendSMS();
        call();
      } else {
        statusOfDay.M_status = 2;
        counter = 0;
      }
    } 
  }
  if(statusOfDay.N_status == 1){
    if(CompareSetupTime(timeSetup.N_hour, timeSetup.N_minute+5)){
      if((digitalRead(Check_Thuoc) == 0) && (counter < 3)){
        notification(2);
        counter++;
      } else if((digitalRead(Check_Thuoc) == 0) && (counter == 2)){
        sendSMS();
        call();
      } else {
        statusOfDay.N_status = 2;
        counter = 0;
      }
    } 
  }
  if(statusOfDay.E_status == 1){
    if(CompareSetupTime(timeSetup.E_hour, timeSetup.E_minute+5)){
      if((digitalRead(Check_Thuoc) == 0) && (counter < 3)){
        notification(2);
        counter++;
      } else if((digitalRead(Check_Thuoc) == 0) && (counter == 2)){
        sendSMS();
        call();
      } else {
        statusOfDay.E_status = 2;
        counter = 0;
      }
    } 
  }
}

/**
 * Read time from internet
*/
void ReadTimerInternet(){
  struct tm timeInfo;
  if(!getLocalTime(&timeInfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  // Serial.print("Month: ");
  // Serial.println(&timeInfo, "%B");
  // Serial.print("Day of Month: ");
  // Serial.println(&timeInfo, "%d");
  // Serial.print("Year: ");
  // Serial.println(&timeInfo, "%Y");

  // Serial.println("Time variables");

  strftime(currentTime._hour,3, "%H", &timeInfo);
  strftime(currentTime._minute,3, "%M", &timeInfo);
  
  // Serial.print(currentTime._hour);
  // Serial.print(":");
  // Serial.print(currentTime._minute);

  // Serial.println();
}

/**
 * Compare time with setup time
*/
bool CompareSetupTime(unsigned char thisHour, unsigned char thisMinute){
  bool status =  false;
  if(thisHour == atoi(currentTime._hour) && thisMinute == atoi(currentTime._minute)){
    status = true;
  }
  return status;
}

void call(){
  char phoneNumber[20] = "0989208486";
  SimSerial.begin(9600);
  SimSerial.println("AT");
  delay(500);
  SimSerial.println(String("ATD+ ")+String(phoneNumber));
  delay(500);
}

void sendSMS(){
  char phoneNumber[20] = "0989208486";
  char message[64] = "Den gio uong thuoc";
  Serial.println("Send SMS");
  SimSerial.begin(9600);
  SimSerial.println("AT");
  delay(500);
  SimSerial.println("AT+CMGF=1");
  delay(500);
  SimSerial.println(String("AT+CMGS=\"") + String(phoneNumber) + String("\""));
  delay(500);
  SimSerial.print(message);
  delay(500);
  SimSerial.write(26);
}
/**
* Hien thi thoi gian tren LCD
*/
void displayTime(){
  lcd.setCursor(0,0);
  lcd.print("--- Time now --- ");
  lcd.setCursor(4,1);
  lcd.print(currentTime._hour);
  lcd.print(" : ");
  lcd.print(currentTime._minute);
}

/**
 * Read data from the EEPROM
*/
void readConfigFromEepRom(){
  timeSetup.M_hour    = EEPROM.read(M_hour_addr);
  timeSetup.M_minute  = EEPROM.read(M_min_addr);
  timeSetup.N_hour    = EEPROM.read(N_hour_addr);
  timeSetup.N_minute  = EEPROM.read(N_min_addr);
  timeSetup.E_hour    = EEPROM.read(E_hour_addr);
  timeSetup.E_minute  = EEPROM.read(E_min_addr);

  Serial.println(String("M: ")+String(timeSetup.M_hour)+String(":")+String(timeSetup.M_minute));
  Serial.println(String("N: ")+String(timeSetup.N_hour)+String(":")+String(timeSetup.N_minute));
  Serial.println(String("E: ")+String(timeSetup.E_hour)+String(":")+String(timeSetup.E_minute));
}

/**
 * Write data to EEPROM
*/
void writeConfigToEepRom(unsigned char data, unsigned char addr){
  EEPROM.write(addr, data);
  EEPROM.commit();
}

/**
 * Reset the EEPROM: write 0 into address
*/
void resetEepRom(){
  for(unsigned char i = 0; i < DayFirst_addr + 12; i++){
    writeConfigToEepRom(0,i);
  }
}

void setup() {

  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("   SYSTEM INIT   ");
  EEPROM.begin(EEPROM_SIZE);
  Initial_Wifi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  ReadTimerInternet();
  Initial_Pin();
  Initial_Data();
  EEPROM.begin(EEPROM_SIZE);
  readConfigFromEepRom();
  Initial_Motor();
  notification(2);
  lcd.setCursor(0,0);
  lcd.print("                ");
}

void loop() {
  displayTime();
  Blynk.run();
  ReadTimerInternet();
  /**
   * Check time morning
  */
  if(CompareSetupTime(timeSetup.M_hour, timeSetup.M_minute) && statusOfDay.M_status == 0) {
      statusOfDay.M_status = 1;
      sendSMS();
      Serial.println("Check morning");
      notification(3);
      delay(1000);
      Motor_goToNextDay(E_dir,E_step,E_button);
  }
  /**
   * Check time noon
  */
  if(CompareSetupTime(timeSetup.N_hour, timeSetup.N_minute) && statusOfDay.N_status == 0) {
    statusOfDay.N_status = 1;
    sendSMS();
    Serial.println("Check Noon");
    notification(3);
    delay(1000);
    Motor_goToNextDay(E_dir,E_step,E_button);
  }
  /**
   * Check time evening
  */
  if(CompareSetupTime(timeSetup.E_hour, timeSetup.E_minute) && statusOfDay.E_status == 0){
    statusOfDay.E_status = 1;
    sendSMS();
    Serial.println("Check Evening");
    notification(3);
    delay(1000);
    Motor_goToNextDay(E_dir,E_step,E_button);
  }
  
  if((statusOfDay.E_status == 1) || (statusOfDay.E_status == 2)){
    if(CompareSetupTime(timeSetup.E_hour, timeSetup.E_minute+1)){
        statusOfDay.M_status = 0;
        statusOfDay.N_status = 0;
        statusOfDay.E_status = 0;
        Initial_Motor();
    } 
  }
 /**
 * Kiem tra xem thuoc con hay khong
 */
 CheckStillMedicine();
}
/**
 * Doc gia tri tu app blynk buoi sang
*/
BLYNK_WRITE(V0) {
  TimeInputParam t(param);
  if (t.hasStartTime())
  {
    Serial.println(String("Morning: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());
    timeSetup.M_hour = t.getStartHour();
    timeSetup.M_minute = t.getStartMinute();
    writeConfigToEepRom(timeSetup.M_hour,M_hour_addr);
    writeConfigToEepRom(timeSetup.M_minute,M_min_addr);
    notification(1);
  }
}
/**
 * Doc gia tri tu app blynk buoi trua
*/
BLYNK_WRITE(V1) {
  TimeInputParam t(param);
  if (t.hasStartTime())
  {
    Serial.println(String("Noon: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());
    timeSetup.N_hour = t.getStartHour();
    timeSetup.N_minute = t.getStartMinute();
    writeConfigToEepRom(timeSetup.N_hour,N_hour_addr);
    writeConfigToEepRom(timeSetup.N_minute,N_min_addr);
    notification(1);
  }
}
/**
 * Doc gia tri tu app blynk buoi toi
*/
BLYNK_WRITE(V2) {
  TimeInputParam t(param);
  if (t.hasStartTime())
  {
    Serial.println(String("Evening: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());
    timeSetup.E_hour = t.getStartHour();
    timeSetup.E_minute = t.getStartMinute();
    writeConfigToEepRom(timeSetup.E_hour,E_hour_addr);
    writeConfigToEepRom(timeSetup.E_minute,E_min_addr);
    notification(1);
  }
}
