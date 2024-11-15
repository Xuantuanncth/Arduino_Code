/*--------------------------------------Library ---------------------------------*/
#include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
/*-------------------------------------------------------------------------------*/

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

#define level_speed_increase 1


#define hour_format 24
#define minute_format 60


/*--------------------------- Define prototype function --------------------------*/
unsigned char reading_button(unsigned char button);
void motor_speed(unsigned char speed);
void change_level_speed(void);
/*--------------------------------------------------------------------------------*/

/*---------------------------- Global variable ----------------------------------*/
DHT dht(dht11_pin,dht_type);
Adafruit_SSD1306 display(screen_width,screen_height,&Wire, oled_reset);

unsigned char level_speed = Off_Speed;
unsigned char mode = 0;
unsigned char auto_mode = 0;
unsigned char swing_mode = 0;
unsigned char start_hour = 0;
unsigned char start_minute = 0;
unsigned char stop_hour = 0;
unsigned char stop_minute = 0;
unsigned char temperature_max = 28;
unsigned char temperature_min = 20;
unsigned char current_temperature = 0;
unsigned char current_hour = 0;
unsigned char current_minute = 0;
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

    /*Initial module DHT 11*/
    dht.begin();

    if(!display.begin(SSD1306_SWITCHCAPVCC, screen_address)) {
        Serial.println(F("SSD1306 allocation failed"));
    }

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
                swing_config();
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

void automatic_mode(){
    Serial.println("Automatic mode");
    bool run_device = false;
    current_temperature = dht.readTemperature();
    /*Check in range of time setup*/
    if(start_hour == 0 && stop_hour == 0 && start_minute == 0 && stop_minute == 0){
        Serial.println("Time range is not set");
        auto_mode = 0;
        return;
    }
    if(current_temperature <= temperature_max){
        Serial.println("Temperature lower than temperature configured");
        return;
    }
    if((start_hour <= current_hour) && (stop_hour >= current_hour))
    {
        if((start_minute <= current_minute) && (stop_minute >= current_minute))
        {
            run_device = true;
        }
    }
    if(run_device){
        motor_speed(Max_Speed);
        motor_swing();
    }
}

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
        // else if(_temp_mode == 1)
        // {
        //     displaySettingTemperature(1);
        //     if(reading_button(btn1) == 1)
        //     {
        //         temperature_min = (temperature_min >= temperature_max)? temperature_max - 1:temperature_min +1;
        //     }
        //     if(reading_button(btn2) == 1)
        //     {
        //         temperature_min = (temperature_min <= 0)? temperature_max - 1:temperature_min -1;
        //     }
        // }
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

    // display.setCursor(15, 35);
    // display.print("MIN: ");
    // display.print(temperature_min);

    display.setTextSize(1);
    display.setCursor(10,55);
    display.print("OK");
    display.setCursor(80, 55);
    display.print("Cancel");

    if(setting_mode == 0){
      display.setCursor(0, 15);
      display.print(">");
    } 
    // else if (setting_mode == 1) {
    //   display.setCursor(0, 35);
    //   display.print(">");
    // } 
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


void temperature_reading()
{
    float h = dht.readHumidity();
    float t = dht.readTemperature();
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

void swing_config(){
    if(reading_button(btn2)==1){
        if(!swing_mode)
        {
            motor_swing();
        }
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

void motor_swing(){
    Serial.println("Motor swing");
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