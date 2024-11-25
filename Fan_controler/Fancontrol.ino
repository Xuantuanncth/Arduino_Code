/*--------------------------------------Library ---------------------------------*/
#include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>
#include <Preferences.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"
#include "RTClib.h"
#include <Arduino.h>
/*-------------------------------------------------------------------------------*/

/*--------------------------------Define variable--------------------------------*/
#define btn1 23
#define btn2 19
#define btn3 18

#define dht11_pin   15
#define motor_pin1  32
#define motor_pin2  33
#define motor_pwm_pin 12
#define motor_stby_pin 13
#define stepper_pin1 25
#define stepper_pin2 26
#define stepper_pin3 27
#define stepper_pin4 14
#define dht11_pin 15

#define Max_Speed 3
#define Normal_Speed 2
#define Min_Speed 1
#define Off_Speed 0

#define dht_type DHT11

#define screen_width 128 // OLED display width, in pixels
#define screen_height 64 // OLED display height, in pixels
#define oled_reset -1
#define screen_address 0x3C

#define start_time 0
#define stop_time  1
#define setting_hour 0
#define setting_minute 1
#define setting_ok 2
#define setting_cancel 3

#define mode_manual  0
#define mode_automatic 1
#define mode_setting_start_time 2
#define mode_setting_stop_time 3
#define mode_setting_temperature 4

#define mode_wifi_waiting 0
#define mode_wifi_connected 1
#define mode_wifi_failed 2
#define mode_wifi_setting 3

#define level_speed_increase 1
/*Sinric pro API key*/
// #define app_key "37fae113-1bd0-490f-83dd-08c0f2a335ed"
// #define app_secret "4aeaf90f-5a26-4b3b-bc43-7007df2a9d03-8738f620-0989-454a-adb4-aaa224ac71d0"
// #define app_device_id "67385a28de70d3c324bd0333"

#define app_key "b965202f-0ba0-44e2-aee1-6093d093fc55"
#define app_secret "e9c5f7f3-c6df-4f87-a224-3d1212d5fd5e-bab95674-fdd6-4a15-8e43-c4ee5cb71518"
#define app_device_id "67386095de70d3c324bd0923"
/*--------------------------------------------------------------------------------*/

/*--------------------------- Define prototype function --------------------------*/
unsigned char reading_button(unsigned char button);
void motor_speed(unsigned char speed);
void change_level_speed(void);
void motor_swing_task(void *pvParameters);
/*--------------------------------------------------------------------------------*/

/*---------------------------- Global variable ----------------------------------*/
DHT dht(dht11_pin,dht_type); 
Adafruit_SSD1306 display(screen_width,screen_height,&Wire, oled_reset);
Preferences preferences;
RTC_DS3231 rtc;
WiFiUDP udp;
TaskHandle_t Swing_stepper_task_handle;

unsigned char level_speed = Off_Speed;
unsigned char mode = 0;
unsigned char auto_mode = 0;
volatile bool swing_mode = false;
unsigned char start_hour = 0;
unsigned char start_minute = 0;
unsigned char stop_hour = 0;
unsigned char stop_minute = 0;
unsigned char temperature_max = 28;
unsigned char temperature_min = 20;
unsigned char current_temperature = 0;
unsigned char current_hour = 0;
unsigned char current_minute = 0;

const char *default_SSID = "Esp32";
const char *default_PASS = "12345678"; //0905772081

bool wifiConnected = false;
const char* ntpServer = "pool.ntp.org";
const int NTP_PACKET_SIZE = 48; 
byte packetBuffer[NTP_PACKET_SIZE]; 

int stepperSequence[8][4] = {
  {1, 0, 0, 1},  // Step 1
  {1, 0, 0, 0},  // Step 2
  {1, 1, 0, 0},  // Step 3
  {0, 1, 0, 0},  // Step 4
  {0, 1, 1, 0},  // Step 5
  {0, 0, 1, 0},  // Step 6
  {0, 0, 1, 1},  // Step 7
  {0, 0, 0, 1}   // Step 8
};
int stepsToMove = 100;
int currentStep = 0;    // To keep track of the current step in the sequence
unsigned long previousMillis = 0;  // Stores the last time the step was updated
const long interval = 10;  // Time between each step (milliseconds)
/*-------------------------------------------------------------------------------*/


void setup() {
    Serial.begin(9600);

    /*Initial button mode*/
    pinMode(btn1, INPUT);
    pinMode(btn2, INPUT);
    pinMode(btn3, INPUT);

    /*Initial motor pin mode*/
    pinMode(motor_pwm_pin, OUTPUT);
    pinMode(motor_stby_pin,OUTPUT);
    pinMode(motor_pin2, OUTPUT);
    pinMode(motor_pin1, OUTPUT);

    pinMode(stepper_pin1, OUTPUT);
    pinMode(stepper_pin2, OUTPUT);
    pinMode(stepper_pin3, OUTPUT);
    pinMode(stepper_pin4, OUTPUT);

    /*Initial module DHT 11*/
    dht.begin();

    if(!display.begin(SSD1306_SWITCHCAPVCC, screen_address)) {
        Serial.println(F("SSD1306 allocation failed"));
    }

    String ssid, password;
    if (loadWiFiCredentials(ssid, password)) {
        Serial.println("WiFi connector loaded from preferences.");
        connectToWiFi( ssid.c_str(),password.c_str());
    }
    if(!wifiConnected){
        Serial.println("WiFi connect with default SSID and password.");
        connectToWiFi(default_SSID,default_PASS);
    }

    if(!wifiConnected){
        selectNewWifi();
    }

    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
    }
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, let's set the time!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    if(wifiConnected){
        setupSinricPro();
        configTime(7 * 3600, 0, ntpServer);
    }

    xTaskCreatePinnedToCore(motor_swing_task, "motor_swing_task", 1000, NULL, 1, &Swing_stepper_task_handle, 0);
    
}

void loop() {
    if(reading_button(btn3) == 1){
        if(mode >= mode_setting_temperature)
        {
          mode = mode_manual;
        }
        else
        {
          mode ++;
        }
    }
    switch(mode)
    {
        case mode_manual:
            displayInfo();
            if(auto_mode){
                automatic_mode();
            } else {
                change_level_speed();
                swing_config(true);
                if(wifiConnected){
                    SinricPro.handle();
                }
            }
            break;
        case mode_automatic:
            setting_automatic_mode();
            break;
        case mode_setting_start_time:
            setting_time_mode(start_time);
            break;
        case mode_setting_stop_time:
            setting_time_mode(stop_time);
            break;
        case mode_setting_temperature:
            setting_temperature_mode();
            break;
    }
}


void connectToWiFi(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) { // Timeout 10s
    delay(500);
    displayWifiSettings(mode_wifi_waiting, setting_ok);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi Connected!");
    displayWifiSettings(mode_wifi_connected, setting_ok);
    delay(2000);
  } else {
    wifiConnected = false;
    Serial.println("\nWiFi Connection Failed!");
  }
}

void displayWifiSettings(unsigned char mode_wifi, unsigned char setting_mode) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 0);  
    display.print("Connect to WiFi");
    display.setTextSize(2);
    display.setCursor(15, 15);
    if(mode_wifi == mode_wifi_waiting) {
        display.print("...");
    } else if (mode_wifi == mode_wifi_connected) {
        display.print("Connected");
    } else if (mode_wifi == mode_wifi_failed) {
        display.print("Failed");

        display.setTextSize(1);
        display.setCursor(10, 40);
        display.print("Select new wifi?");
        display.setCursor(10,55);
        display.print("OK");
        display.setCursor(80, 55);
        display.print("Cancel");

        if (setting_mode == setting_ok) {
            display.setCursor(0, 55);
            display.print(">");
        } else {
            display.setCursor(70, 55);
            display.print(">");
        }
    } else {
        display.setTextSize(1);
        display.setCursor(0, 15);
        display.print("AP: ESP32_Config");
        display.setCursor(0, 25);
        display.print("Pass: 12345678");
        display.setCursor(0, 35);
        display.print("http://192.168.4.1");

    }
    display.display();
}

void selectNewWifi() {
    unsigned char _temp_mode = 0;
    bool is_get_wifi = false;
    while(true){
        if(reading_button(btn3) == 1) {
            _temp_mode = (_temp_mode >= 1) ? 0: _temp_mode+1;
        }
        if(_temp_mode == 0)
        {
            displayWifiSettings(mode_wifi_failed, setting_ok);
            if(reading_button(btn1) == 1){
                is_get_wifi = true;
                break;
            }
        } 
        else
        {
            displayWifiSettings(mode_wifi_failed, setting_cancel);
            if(reading_button(btn1) == 1){
                break;
            }
        }
    }
    if(is_get_wifi){
        createAccessPoint();
    }
}

void createAccessPoint() {
    const char* apSSID = "ESP32_Config";
    const char* apPassword = "12345678";

    WiFi.softAP(apSSID, apPassword);
    Serial.println("Access Point Created");
    displayWifiSettings(mode_wifi_setting, setting_ok);

    WiFiServer server(80);
    server.begin();

    while (true) {
        WiFiClient client = server.available();
        if (client) {
            Serial.println("New Client Connected");
            String request = client.readStringUntil('\r');
            client.flush();

            if (request.indexOf("GET /") >= 0) {
                String html = "<html><body>"
                                "<h1>WiFi Setup</h1>"
                                "<form method='GET' action='/setwifi'>"
                                "SSID: <input type='text' name='ssid'><br>"
                                "Password: <input type='text' name='password'><br>"
                                "<input type='submit' value='Submit'>"
                                "</form>"
                                "<a href='/cancel'><button>Cancel</button></a>"
                                "</body></html>";
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println();
                client.println(html);
                client.println();
            }

            if (request.indexOf("GET /setwifi") >= 0) {
                int ssidIndex = request.indexOf("ssid=") + 5;
                int passIndex = request.indexOf("password=") + 9;

                String newSSID = request.substring(ssidIndex, request.indexOf('&', ssidIndex));
                String newPassword = request.substring(passIndex, request.indexOf(' ', passIndex));

                newSSID.replace("+", " ");
                newSSID.replace("%20", " "); 
                newPassword.replace("%20", " ");

                Serial.println("New WiFi Credentials:");
                Serial.println("SSID: " + newSSID);
                Serial.println("Password: " + newPassword);

                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/plain");
                client.println();
                client.println("Trying to connect...");
                client.println();

                connectToWiFi(newSSID.c_str(), newPassword.c_str());
                if (wifiConnected) {
                    saveWiFiCredentials(newSSID, newPassword);
                    break;
                } else {
                    displayWifiSettings(mode_wifi_failed, setting_ok);
                    break;
                }
            }
            if (request.indexOf("GET /cancel") >= 0) {
                break;
            }
        }
    }
}

void saveWiFiCredentials(String ssid, String password) {
    preferences.begin("wifi", false);  
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end(); 
    Serial.println("WiFi credentials saved!");
}

bool loadWiFiCredentials(String &ssid, String &password) {
    preferences.begin("wifi", true); 
    ssid = preferences.getString("ssid", "");  
    password = preferences.getString("password", "");
    Serial.println("WiFi of Credentials loaded");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
    preferences.end();
    return ssid.length() > 0 && password.length() > 0; 
}

void setupSinricPro() {
  // add devices and callbacks to SinricPro
  SinricProSwitch& mySwitch = SinricPro[app_device_id];
  mySwitch.onPowerState(onPowerState);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(app_key, app_secret);
}

bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Device %s turned %s\n", deviceId.c_str(), state ? "ON" : "OFF");
  if(state) {
    level_speed = Max_Speed;
  } else {
    level_speed = Off_Speed;
  }
  return true; 
}

/**
 * Automatically controls the fan based on the current temperature and time settings.
 * 
 * This function checks if the current temperature is within the configured temperature range and if the current time is within the configured start and stop time. If both conditions are met, the function sets the motor speed to the maximum speed and activates the swing mode.
 * 
 * If the time range is not set (all values are 0), the function prints a message and disables the automatic mode.
 * 
 * If the current temperature is lower than the configured maximum temperature, the function returns without taking any action.
 */
void automatic_mode(){
    Serial.println("Automatic mode");
    bool run_device = false;
    int range = 0;

    if(wifiConnected){
        struct tm timeInfo;
        if (getLocalTime(&timeInfo)) {
            current_hour = timeInfo.tm_hour;
            current_minute = timeInfo.tm_min;
        }
        else
        {
          Serial.println("Failed to get date time from internet");
        }
    } 
    else 
    {
        DateTime now = rtc.now();
        current_hour = now.hour();
        current_minute = now.minute();
    }
    current_temperature = dht.readTemperature();
    Serial.print("Current time: ");
    Serial.print(current_hour);
    Serial.print(':');
    Serial.println(current_minute);
    /*Check in range of time setup*/
    if(start_hour == 0 && stop_hour == 0 && start_minute == 0 && stop_minute == 0){
        Serial.println("Time range is not set");
        auto_mode = 0;
        return;
    }
    if((current_temperature > temperature_max) || (current_temperature < temperature_min)){
        Serial.println("Temperature lower than temperature configured");
        return;
    }
    /*
      Settime in a day
    */
    if((start_hour <= current_hour) && (stop_hour >= current_hour)) 
    {
        if((start_minute <= current_minute) && (stop_minute >= current_minute))
        {
            run_device = true;
            swing_mode = true;
        }
    }
    if(run_device){
        range = (temperature_max - temperature_min)/3;
        if(current_temperature < temperature_min + range)
        {
          level_speed = Min_Speed;
        }
        else if(((temperature_min + range) <= current_temperature)&&(current_temperature < (temperature_max + range)))
        {
          level_speed = Normal_Speed;
        } 
        else if(current_temperature > temperature_max - range)
        {
          level_speed = Max_Speed;
        }
        motor_speed(level_speed);
        swing_config(true);
    } else {
        motor_speed(Off_Speed);
        swing_config(false);
    }
}

/**
 * Configures the start or stop time settings for the fan control system's automatic mode.
 * 
 * This function handles the user interface for setting the start or stop time for the automatic mode. It displays a menu with options to adjust the hour and minute, as well as OK and Cancel options.
 * 
 * The function runs in a loop until the user makes a selection:
 * - Button 3 toggles between the hour, minute, OK, and Cancel options
 * - Button 1 increases the selected value (hour or minute)
 * - Button 2 decreases the selected value (hour or minute)
 * - Selecting OK saves the configured time and moves to the next setting (start time -> stop time, or stop time -> temperature)
 * - Selecting Cancel exits the time setting mode and returns to manual mode
 * 
 * @param cfg_mode The configuration mode, either start_time or stop_time, to set the time for.
 */
void setting_time_mode(unsigned int cfg_mode) {
    Serial.println("Setting time mode");
    unsigned char _mode_temp = 0;
    unsigned char _hour_temp = 0;
    unsigned char _min_temp = 0;
    unsigned char _config_mode = cfg_mode;
    if(_config_mode == start_time){
        _hour_temp = start_hour;
        _min_temp = start_minute;
    } else {
        _hour_temp = stop_hour;
        _min_temp = stop_minute;
    }
    while(true)
    {
        if(reading_button(btn3) == 1)
        {
            _mode_temp = (_mode_temp >= 3) ? 0: _mode_temp+1;
        }
        if(_mode_temp == 0)
        {
            displaySettingTime(_config_mode,setting_hour);
            if(reading_button(btn1) == 1)
            {
                _hour_temp = (_hour_temp >= 23) ? 0 : _hour_temp +1;
            }
            if(reading_button(btn2) == 1)
            {
                _hour_temp = (_hour_temp <= 0) ? 23 : _hour_temp -1;
            }
            if(_config_mode == start_time){
                start_hour = _hour_temp;
            } else {
                stop_hour = _hour_temp;
            }
        } 
        else if(_mode_temp == 1)
        {
            displaySettingTime(_config_mode, setting_minute);
            if(reading_button(btn1) == 1)
            {
                _min_temp = (_min_temp >= 59) ? 0 : _min_temp +1;
            }
            if(reading_button(btn2) == 1)
            {
                _min_temp = (_min_temp <= 0) ? 59 : _min_temp -1;
            }
            if(_config_mode == start_time){
                start_minute = _min_temp;
            } else {
                stop_minute = _min_temp;
            }
        }
        else if (_mode_temp == 2)
        {
            displaySettingTime(_config_mode, setting_ok);
            if(reading_button(btn1) == 1){
                if(_config_mode == start_time){
                    mode = mode_setting_stop_time;
                } else {
                    mode = mode_setting_temperature;
                }
                break;
            }
        }
        else {
            displaySettingTime(_config_mode, setting_cancel);
            if(reading_button(btn1) == 1){
                mode = mode_manual;
                break;
            }
        }
    }
}

/**
 * Configures the automatic mode settings for the fan control system.
 * 
 * This function handles the user interface for enabling/disabling automatic mode.
 * It displays a menu with OK/Cancel options and processes button inputs to:
 * - Enable automatic mode and proceed to start time settings when OK is selected
 * - Disable automatic mode and return to manual mode when Cancel is selected
 * 
 * The function runs in a loop until the user makes a selection:
 * - Button 3 toggles between OK and Cancel options
 * - Button 1 confirms the current selection
 * 
 * When OK is selected:
 * - Sets auto_mode to 1 (enabled)
 * - Changes mode to mode_setting_start_time
 * 
 * When Cancel is selected:
 * - Sets auto_mode to 0 (disabled) 
 * - Changes mode back to mode_manual
 */
void setting_automatic_mode(){
    Serial.println("Automatic mode");
    unsigned char _temp_mode = 0;
    while(true)
    {
        if(reading_button(btn3) == 1) {
            _temp_mode = (_temp_mode >= 1) ? 0: _temp_mode+1;
        }
        if(_temp_mode == 0)
        {
            displaySettingAutomatic(setting_ok);
            if(reading_button(btn1) == 1){
                auto_mode = 1;
                mode = mode_setting_start_time;
                break;
            }
        } else
        {
            displaySettingAutomatic(setting_cancel);
            if(reading_button(btn1) == 1){
                auto_mode = 0;
                mode = mode_manual;
                break;
            }
        }
    }
}

void setting_temperature_mode(){
    Serial.println("Setting temperature mode");
    unsigned char _temp_mode = 0;
    while(true)
    {
        if(reading_button(btn3) == 1)
        {
            _temp_mode = (_temp_mode >= 2) ? 0: _temp_mode+1;
        }
        if(_temp_mode == 0)
        {
            displaySettingTemperature(0);
            if(reading_button(btn1) == 1)
            {
                temperature_max = (temperature_max >= 45)? 20:temperature_max +1;
            }
            if(reading_button(btn2) == 1)
            {
                temperature_max = (temperature_max <= 20)? 45:temperature_max -1;
            }
        } 
        else if(_temp_mode == 1)
        {
            displaySettingTemperature(1);
            if(reading_button(btn1) == 1)
            {
                temperature_min = (temperature_min >= temperature_max)? temperature_max - 3: temperature_min +1;
            }
            if(reading_button(btn2) == 1)
            {
                temperature_min = (temperature_min <= 0)? temperature_max - 1 : temperature_min -1;
            }
        }
        else if (_temp_mode == 1)
        {
            displaySettingTemperature(setting_ok);
            if(reading_button(btn1) == 1){
                mode = mode_manual;
                break;
            }
        }
        else {
            displaySettingTemperature(setting_cancel);
            if(reading_button(btn1) == 1){
                mode = mode_manual;
                break;
            }
        }
    }
}

void displaySettingAutomatic(unsigned char setting_mode){
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 0);  
    display.print("Setting Automatic");

    display.setTextSize(2);
    display.setCursor(15, 15);
    display.print("AutoMode");

    display.setTextSize(1);
    display.setCursor(10,55);
    display.print("OK");
    display.setCursor(80, 55);
    display.print("Cancel");

    if (setting_mode == setting_ok) {
        display.setCursor(0, 55);
        display.print(">");
    } else {
        display.setCursor(70, 55);
        display.print(">");
    }

    display.display();
}

void displaySettingTemperature(unsigned char setting_mode){
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 0);  
    display.print("Setting Temperature");

    display.setTextSize(2);
    display.setCursor(15, 15);
    display.print("MAX: ");
    display.print(temperature_max);

    display.setCursor(15, 35);
    display.print("MIN: ");
    display.print(temperature_min);

    display.setTextSize(1);
    display.setCursor(10,55);
    display.print("OK");
    display.setCursor(80, 55);
    display.print("Cancel");

    if(setting_mode == 0){
      display.setCursor(0, 15);
      display.print(">");
    } 
    else if (setting_mode == 1) {
      display.setCursor(0, 35);
      display.print(">");
    } 
    else if (setting_mode == setting_ok) {
        display.setCursor(0, 55);
        display.print(">");
    } else {
        display.setCursor(70, 55);
        display.print(">");
    }

    display.display();
}

void displayInfo() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1, 0);  
    display.print("Temperature: ");
    display.print(temperature);
    display.print(" ");
    display.write(248);
    display.print("C");

    display.setCursor(1, 10);  
    display.print("Humidity: ");
    display.print(humidity);
    display.print(" %");

    display.setTextSize(3);
    display.setCursor(55, 30);
    display.print(level_speed);

    display.setTextSize(1);
    display.setCursor(80, 55);
    display.print("Auto:");
    if(auto_mode == 1){
      display.print("On");
    } else {
      display.print("Off");
    }
    display.display();
}

void displaySettingTime(unsigned char setting_time, unsigned char setting_mode) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(10, 0);
    if (setting_time == start_time)
    {        
      display.print("Setting Start Time");
    } else 
    {
      display.print("Setting Stop Time");
    }

    display.setTextSize(2);
    display.setCursor(15, 15);
    display.print("H: ");
    if(setting_time == start_time){
      display.print(start_hour);
    } else {
      display.print(stop_hour);
    }



    display.setCursor(15, 35);
    display.print("M: ");
    if(setting_time == start_time){
      display.print(start_minute);
    } else {
      display.print(stop_minute);
    }

    display.setTextSize(1);
    display.setCursor(10,55);
    display.print("OK");
    display.setCursor(80, 55);
    display.print("Cancel");

    if(setting_mode == setting_hour){
      display.setCursor(0, 15);
      display.print(">");
    } else if (setting_mode == setting_minute) {
      display.setCursor(0, 35);
      display.print(">");
    } else if (setting_mode == setting_ok) {
        display.setCursor(0, 55);
        display.print(">");
    } else {
        display.setCursor(70, 55);
        display.print(">");
    }

    display.display();
}

void change_level_speed()
{
  unsigned char button_press = reading_button(btn1);
  if(button_press){
    if(level_speed >= Max_Speed){
      level_speed = Off_Speed;
    } else {
      level_speed ++;
    }
  }
  motor_speed(level_speed);
}

void swing_config(unsigned char swing){
    if(true == swing){
        if(reading_button(btn2)==1){
            swing_mode = (swing_mode == true) ? false: true;
        }
    } else {
      swing_mode = false;
    }
}

unsigned char reading_button(unsigned char button){
  unsigned char state = 0;
  if(digitalRead(button) == 0){
    delay(50);
    state = 1;
    while(digitalRead(button) == 0);
  }
  return state;
}

void motor_swing_task(void *pvParameters) {
    while(true){
        if(swing_mode == true){
            Serial.println("Motor swing");
            // Move forward 1/3 of a revolution
            for (int i = 0; i < stepsToMove; i++) {
                if(swing_mode){
                    for (int step = 0; step < 8; step++) {
                    digitalWrite(stepper_pin1, stepperSequence[step][0]);
                    digitalWrite(stepper_pin2, stepperSequence[step][1]);
                    digitalWrite(stepper_pin3, stepperSequence[step][2]);
                    digitalWrite(stepper_pin4, stepperSequence[step][3]);
                    delay(10);  // Small delay between steps to control speed
                    }
                }else
                {
                    break;
                }
            }

            delay(500);  // Wait 1 second before returning

            // Move backward 1/3 of a revolution (back to original position)
            for (int i = 0; i < stepsToMove; i++) {
                if(swing_mode){
                    for (int step = 7; step >= 0; step--) {
                    digitalWrite(stepper_pin1, stepperSequence[step][0]);
                    digitalWrite(stepper_pin2, stepperSequence[step][1]);
                    digitalWrite(stepper_pin3, stepperSequence[step][2]);
                    digitalWrite(stepper_pin4, stepperSequence[step][3]);
                    delay(10);  // Small delay between steps to control speed
                    }
                }else{
                    break;
                }
            }
            delay(500);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void motor_speed(unsigned char speed){

  if(speed != Off_Speed){
    digitalWrite(motor_pin1, HIGH);
    digitalWrite(motor_pin2, LOW);
    if(speed == Max_Speed){
        analogWrite(motor_pwm_pin, 255);
    } else if(speed == Normal_Speed){
        analogWrite(motor_pwm_pin, 220);
    } else{
        analogWrite(motor_pwm_pin, 190);
    } 
  } else {
    digitalWrite(motor_pin1, LOW);
    digitalWrite(motor_pin2, LOW);
  }
}