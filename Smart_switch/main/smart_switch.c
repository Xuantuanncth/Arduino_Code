#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <WiFi.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"

#include <Wire.h>
#include <LiquidCrystal.h>

// Define PIN

#define Relay1 4
#define Relay2 16
#define Button1 19
#define Button2 18

#define OUTPUT 0
#define INPUT 1

// Define wifi and authentication
#define WIFI_SSID         "smart_switch"    
#define WIFI_PASS         "12345678"

#define APP_KEY           "9e7e4546-27ec-4050-8ad8-d7ae695416bf"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "dbae38dd-6551-416d-95aa-d1a12f6b2c29-7918585b-fdb1-4b4d-861b-0a99648ef4b5"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"

#define SWITCH_ID_1       "6550a04a93a1f6be5eaf2e51"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define SWITCH_ID_2       "6550a09193a1f6be5eaf2e7b"    // Should look like "5dc1564130xxxxxxxxxxxxxx"

#define STATE_ON  1
#define STATE_OFF 0

unsigned int state_device1 = STATE_OFF;
unsigned int state_device2 = STATE_OFF;

const int rs = 23, rw = 4, en = 22, d4 = 5, d5 = 18, d6 = 19, d7 = 21;
LiquidCrystal lcd(rs, rw, en, d4, d5, d6, d7);

static delay(unsigned int time){
    for(int i = 0; i < time; i++){
        for(int j = 0; j< 100; j++)
        {}
    }
}

static void display_lcd(){
  lcd.setCursor(0,0);
  lcd.print("Device1: ");
  state_device1 ? lcd.print("ON ") : lcd.print("OFF");
  lcd.setCursor(0,1);
  lcd.print("Device2: ");
  state_device2 ? lcd.print("ON ") : lcd.print("OFF");
}

static bool onPowerState1(const String &deviceId, bool &state) {
  state_device1 = state;
  return true; // request handled properly
}

static bool onPowerState2(const String &deviceId, bool &state) {
  state_device2 = state;
  return true; // request handled properly
}

static void setupWiFi() {


  WiFi.setSleep(false); 
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS); 

  while (WiFi.status() != WL_CONNECTED) {
    printf(".");
    delay(250);
  }

  printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

static void setupSinricPro() {
  // add devices and callbacks to SinricPro
  SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];
  mySwitch1.onPowerState(onPowerState1);

  SinricProSwitch& mySwitch2 = SinricPro[SWITCH_ID_2];
  mySwitch2.onPowerState(onPowerState2);
  
  // setup SinricPro
  SinricPro.onConnected([](){ printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

static void init_mode(){

    gpio_set_level(Relay1, OUTPUT);
    gpio_set_level(Relay2, OUTPUT);
    gpio_set_level(Button1, INPUT);
    gpio_set_level(Button2, INPUT);

    lcd.init();
    lcd.backlight();
    lcd.setCursor(3,0);
    lcd.print("System Init");
    lcd.setCursor(0,1);
    lcd.print("Connect wifi");
    .printf("\r\n\r\n");
    setupWiFi();
    setupSinricPro();
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Welcome");
    delay(2000);
    lcd.clear();

}

void app_main(void)
{
  
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);
    init_mode();
    while(1) {
        SinricPro.handle();
        if(digitalRead(Button1)){
            delay(50);
            state_device1 = !state_device1;
            while(digitalRead(Button1));
        }
        if(digitalRead(Button2)){
            delay(50);
            state_device2 = !state_device2;
            while(digitalRead(Button2));
        }
        state_device1 ? digitalWrite(Relay1,LOW) : digitalWrite(Relay1,HIGH);
        state_device2 ? digitalWrite(Relay2,LOW) : digitalWrite(Relay2,HIGH);
        display_lcd();
    }

}