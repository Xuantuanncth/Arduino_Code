/*--------------------------------- FingerPrinter with ESP32 ------------------------------*/
/*
*
*
*/
/*--------------------------------- Include Lirary ---------------------------------------*/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include "logo.h"
#include "RTClib.h"
#include <EEPROM.h>
/*---------------------------------------------------------------------------------------*/

/*--------------------------------- Define Variable -------------------------------------*/
#define buzz_pin 4

/*Variable for LCD*/
#define screen_width 128
#define screen_height 64
#define oled_reset -1
#define screen_address 0x3C

#define notif_success 1
#define notif_error 2

#define mode_first_add_fingerprint 1
#define mode_second_add_fingerprint 2
#define mode_success_add_fingerprint 3
#define mode_error_add_fingerprint 4
#define mode_get_id 5
#define mode_confirm 6
#define mode_success 7
#define mode_error 8

#define mode_setting_none 1
#define mode_setting_menu 2
#define mode_setting_add_finger 3
#define mode_setting_delete_finger 4
#define mode_setting_time 5
#define mode_setting_list_finger 6
#define mode_setting_clear_database 7

#define finger_detect_none 1
#define finger_detect_success 2
#define finger_detect_error 3

#define eeprom_size 512 
/*---------------------------------------------------------------------------------------*/

/*--------------------------------- Global Variable -------------------------------------*/
Adafruit_SSD1306 display(screen_width,screen_height,&Wire, oled_reset);
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger(&mySerial);
RTC_DS1307 rtc;

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns

char hexaKeys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {33, 12, 14, 26}; 
byte colPins[COLS] = {25, 32, 27};

Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
unsigned char mode = mode_setting_none;

unsigned char id_checking = 0;
/*Variable of time setting*/
unsigned char in_hour = 8; /*Start time of working day*/
unsigned char in_minute = 0;
unsigned char out_hour = 17; /*End time of working day*/
unsigned char out_minute = 0;

unsigned char current_hour = 0;
unsigned char current_min = 0;
unsigned char current_sec = 0;

const char* status[] = {"IN_OK", "IN_LATE", "OUT_SOON", "OUT_OK"};
char current_date[16];
char current_time[10];
char message[40];

/*If use debug_app it'll be true*/
bool debug_app = true;
/*---------------------------------------------------------------------------------------*/


void setup() {
    Serial.begin(115200);
    mySerial.begin(57600, SERIAL_8N1, 16, 17);
    finger.begin(57600);
    EEPROM.begin(eeprom_size);

    if (finger.verifyPassword()) {
      Serial.println("Found fingerprint sensor!");
    } else {
      Serial.println("Did not find fingerprint sensor :(");
      // while (1) { delay(1); }
    }

    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
    Serial.print(F("Security level: ")); Serial.println(finger.security_level);
    Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

    pinMode(buzz_pin, OUTPUT);

    if(!display.begin(SSD1306_SWITCHCAPVCC, screen_address)) {
        Serial.println(F("SSD1306 allocation failed"));
    }

    display_welcome(0);
    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
    }

}

void loop() {
    char key = keypad.getKey();
    if (key == '*'){
        // Serial.println("Switch mode");
        mode = mode_setting_menu;
    }
    switch(mode) {
        case mode_setting_none:
            check_fingerprint();
            display_welcome(1);
            get_time_from_rtd();  
            break;
        case mode_setting_menu:
            setting_menu_mode();
            break;
        case mode_setting_add_finger:
            add_fingerprint_mode();
            break;
        case mode_setting_delete_finger:
            delete_fingerprint_mode();
            break;
        case mode_setting_time:
            setting_time_mode();
            break;
        case mode_setting_list_finger:
            show_list_finger();
            break;
        case mode_setting_clear_database:
            setting_clear_database();
            break;
    }
}

void get_time_from_rtd(){
  DateTime now = rtc.now();
  current_hour = now.hour();
  current_min = now.minute();
  current_sec = now.second();
  snprintf(current_date, sizeof(current_date), "%02d/%02d/%04d", now.day(), now.month(), now.year());
  snprintf(current_time, sizeof(current_time), "%02d:%02d:%02d", current_hour, current_min, current_sec);
}

void send_data_to_application()
{
  /*
  * Format *#3_19/11/2023_10:50:07_IN_LATE#
  * '*#'          : Start string
  * '3'           : ID
  * '19/11/2023'  : Date
  * '10:50:07'    : Time
  * 'IN_LATE'     : status
  */
  if (current_hour < in_hour || (current_hour == in_hour && current_min <= in_minute)) {
    snprintf(message, sizeof(message), "*#%d_%s_%s_%s#", id_checking, current_date, current_time, status[0]); // IN_OK
  } else if (current_hour >= in_hour && current_hour < 12 && current_min > in_minute) {
    snprintf(message, sizeof(message), "*#%d_%s_%s_%s#", id_checking, current_date, current_time, status[1]); // IN_LATE
  } else if (current_hour < out_hour || (current_hour == out_hour && current_min < out_minute)) {
    snprintf(message, sizeof(message), "*#%d_%s_%s_%s#", id_checking, current_date, current_time, status[2]); // OUT_SOON
  } else if (current_hour >= out_hour && current_min >= out_minute) {
    snprintf(message, sizeof(message), "*#%d_%s_%s_%s#", id_checking, current_date, current_time, status[3]); // OUT_OK
  }
  Serial.print(message);
}

void setting_menu_mode()
{
    unsigned char _mode_temp = 0;
    unsigned char page = 0;
    while(1)
    {
        char key = keypad.getKey();
        if(key == '*')
        {
            if(page == 0)
            {
                _mode_temp = (_mode_temp>=4) ? 0 : _mode_temp + 1 ;
            }
            else if(page == 1)
            {
                _mode_temp = (_mode_temp>=3) ? 0 : _mode_temp + 1 ;
            }
        } 

        display_menu_mode(page, _mode_temp);

        if( (key == '#') && (page == 0) && (_mode_temp == 0)){
            mode = mode_setting_add_finger;
            break;
        }
        else if((key == '#') && (page == 0) && (_mode_temp == 1)){
            mode = mode_setting_delete_finger;
            break;
        }
        else if((key == '#') && (page == 0) && (_mode_temp == 2)){
            mode = mode_setting_time;
            break;
        }
        else if((key == '#') && (page == 0) && (_mode_temp == 3)){ // Press to next
            _mode_temp = 0;
            page = 1;
        }
        else if ((key == '#') && (page == 1) && (_mode_temp == 0))  
        {
            mode = mode_setting_list_finger;
            break;
        }
        else if ((key == '#') && (page == 1) && (_mode_temp == 1))  
        {
            mode = mode_setting_clear_database;
            break;
        }
        else if ((key == '#') && (page == 1) && (_mode_temp == 2)) // Press to back
        {
            _mode_temp = 0;
            page = 0;
        }
        else if((key == '#') && ((_mode_temp == 4) || ((page == 1) && (_mode_temp == 3)))){
            mode = mode_setting_none;
            break;
        }
    }
}

void display_menu_mode(unsigned char page, unsigned char mode_menu)
{
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 0);
    display.print("Setting");
    display.setTextSize(1);
    if(page == 0){
        display.setCursor(15,25);
        display.print("1: Add Finger");
        display.setCursor(15,35);
        display.print("2: Del Finger");
        display.setCursor(15,45);
        display.print("3: Setting time");
        display.setCursor(15,55);
        display.print("Next");
        display.setCursor(80,55);
        display.print("Cancel");

        if(mode_menu == 0){
            display.setCursor(5,25);
            display.print(">");
        } 
        else if(mode_menu == 1){
            display.setCursor(5,35);
            display.print(">");
        }
        else if(mode_menu == 2){
            display.setCursor(5,45);
            display.print(">");
        } 
        else if(mode_menu == 3){
            display.setCursor(5,55);
            display.print(">");
        }
        else if(mode_menu == 4){
            display.setCursor(70,55);
            display.print(">");
        }
    } 
    else if (page == 1){
        display.setCursor(15,25);
        display.print("1: List Finger");
        display.setCursor(15,35);
        display.print("2: Del Database");

        display.setCursor(15,55);
        display.print("Back");
        display.setCursor(80,55);
        display.print("Cancel");

        if(mode_menu == 0){
            display.setCursor(5,25);
            display.print(">");
        }
        else if(mode_menu == 1){
            display.setCursor(5,35);
            display.print(">");
        } 
        else if(mode_menu == 2){
            display.setCursor(5,55);
            display.print(">");
        }
        else if(mode_menu == 3){
            display.setCursor(70,55);
            display.print(">");
        }
    }

    display.display();
}

void display_setting_time(unsigned char mode_display){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(5,0);
    display.print("-- Setting Time --");
    display.setTextSize(1);

    display.setCursor(15,20);
    display.print("H_in: ");
    display.setCursor(50,20);
    display.print(in_hour);
    display.setCursor(75,20);
    display.print("M_in: ");
    display.setCursor(115,20);
    display.print(in_minute);

    display.setCursor(15,35);
    display.print("H_out: ");
    display.setCursor(50,35);
    display.print(out_hour);
    display.setCursor(75,35);
    display.print("M_out: ");
    display.setCursor(115,35);
    display.print(out_minute);

    display.setCursor(15,55);
    display.print("Ok");

    if(mode_display == 0)
    {
        display.setCursor(5,20);
        display.print(">");
    } 
    else if(mode_display == 1)
    {
        display.setCursor(65,20);
        display.print(">");
    } 
    else if(mode_display == 2)
    {
        display.setCursor(5,35);
        display.print(">");
    }
    else if(mode_display == 3)
    {
        display.setCursor(65,35);
        display.print(">");
    }
    else if(mode_display == 4){
        display.setCursor(5,55);
        display.print(">");
    }
    else
    {
      //Do nothing
    }

    display.display();
}

unsigned char input_number(char key, unsigned char current_number, String *inputNumber)
{
    unsigned char ret_key = current_number;

    if (key >= '0' && key <= '9') 
    {
        *inputNumber += key;
        if(debug_app)
        {
            Serial.print("Current input: ");
            Serial.println(*inputNumber);
        }
    } 
    else if (key == '#') 
    {
        if (inputNumber->length() > 0) 
        {
            ret_key = inputNumber->toInt();
            *inputNumber = "";
        }
        if(debug_app)
        {
            Serial.print("Number: ");
            Serial.println(ret_key);
        }
    }

    return ret_key;
}


void setting_time_mode(){
    unsigned char _mode = 0;
    String inputNumber = "";
    while(true){
        char key = keypad.getKey();
        display_setting_time(_mode);
        if(_mode == 0)
        {
          in_hour = input_number(key, in_hour, &inputNumber);
          if(in_hour > 23)
          {
            in_hour = 0;
          }
        }
        else if (_mode == 1)
        {
            in_minute = input_number(key, in_minute, &inputNumber);
            if(in_minute > 59)
            {
                in_minute = 0;
            }
        }
        else if (_mode == 2)
        {
            out_hour = input_number(key, out_hour, &inputNumber);
            if(out_hour > 23)
            {
                out_hour = 0;
            }
        }
        else if (_mode == 3)
        {
            out_minute = input_number(key, out_minute, &inputNumber);
            if(out_minute > 59)
            {
                out_minute = 0;
            }
        }
        else if (_mode == 4 && key =='#')
        {
            mode = mode_setting_none;
            break;
        }
        else
        {
            //Do nothing
        }
        if(key == '*'){
            _mode = (_mode >= 4 ) ? 0 : _mode + 1 ;
            inputNumber = "";
        }
        delay(10);
    }

}

void display_welcome(unsigned char mode){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(15,0);
    display.print("Welcome!");
    if(mode == 0){
        display.setCursor(15,25);
        display.print("Loading..");
    } 
    else 
    {
        display.setCursor(5,25);
        display.print(current_date);
        display.setCursor(15,45);
        display.print(current_time);
    }
    display.display();
}

void display_log(const uint8_t *logo) {
    display.clearDisplay();

    display.drawBitmap(
                        (display.width()  - logo_width ) / 2,
                        (display.height() - logo_height) / 2,
                        logo, logo_width, logo_width, 1);
    display.display();
    delay(1000);
}

void display_mode_add_fingerprint (unsigned char mode_display, String id)
{
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 0);
    display.print("Add Finger");
    display.setTextSize(1);

    if( mode_display == mode_get_id){
        display.setTextSize(2);
        display.setCursor(20,35);
        display.print("ID: ");
        display.print(id);
    } 
    else if( mode_display == mode_first_add_fingerprint)
    {
        display.setCursor(15,35);
        display.print("Put Finger first");
    } 
    else if( mode_display == mode_second_add_fingerprint)
    {
        display.setCursor(15,35);
        display.print("Put Finger again");
    } 
    else if ( mode_display == mode_success_add_fingerprint)
    {
        display.setTextSize(2);
        display.setCursor(25,35);
        display.print("Success!");
    } 
    else if ( mode_display == mode_error_add_fingerprint)
    {
        display.setTextSize(2);
        display.setCursor(25,35);
        display.print("Error!");
    }
    else
    {
        // Do nothing
    }
    display.display();
}

void notification(unsigned char notification_type){
    unsigned char i = 0;
    if(notification_type == notif_success) {
        for(i = 0; i< 2; i++) {
            digitalWrite(buzz_pin, HIGH);
            delay(100);
            digitalWrite(buzz_pin, LOW);
            delay(100);
        }
    } else if(notification_type == notif_error) {
        for(i = 0; i< 3; i++) {
            digitalWrite(buzz_pin, HIGH);
            delay(100);
            digitalWrite(buzz_pin, LOW);
            delay(100);
        }
    }
}

unsigned char detect_fingerprint()
{
    unsigned char is_fingerprint = finger_detect_none;
    char return_val = -1;
    return_val = finger.getImage();

    if(return_val == FINGERPRINT_OK){

        return_val = finger.image2Tz();

        if(return_val == FINGERPRINT_OK)
        {
            return_val = finger.fingerFastSearch();

            if(return_val == FINGERPRINT_OK)
            {
                is_fingerprint = finger_detect_success;
                id_checking = finger.fingerID;
                if(debug_app)
                {
                  Serial.print("Fingerprint ID: ");
                  Serial.println(id_checking);
                }
                // storeFingerprintData(id);
            }
            else
            {
                is_fingerprint = finger_detect_error;
            } 
        } 
    } 

    return is_fingerprint;  
}

void check_fingerprint(){
    unsigned char is_fingerprint = detect_fingerprint();
    if(is_fingerprint == finger_detect_success)
    {   
        send_data_to_application();
        display_log(success_logo);
        notification(notif_success);
    } 
    else if(is_fingerprint == finger_detect_error)
    {
        display_log(error_logo);
        notification(notif_error);
    }
    else{
        // Do nothing
    }
}

bool enroll_fingerprint(unsigned char id_fingerprint, unsigned char mode_enroll)
{
    char return_val = -1;
    bool status = false;
    unsigned int time_out = 2000;
    while(time_out > 0)
    {
        return_val = finger.getImage();

        if(return_val == FINGERPRINT_OK)
        {
            if(mode_enroll == mode_first_add_fingerprint)
            {
                return_val = finger.image2Tz(1);
            }
            else if(mode_enroll == mode_second_add_fingerprint)
            {
                return_val = finger.image2Tz(2);
            }

            if(return_val == FINGERPRINT_OK)
            {
                status = true;
            }
        }

        if((mode_enroll == mode_first_add_fingerprint)&&(status == true))
        {
            // Do no thing
            break;
        } 
        else if ((mode_enroll == mode_second_add_fingerprint)&&(status == true))
        {
            return_val = finger.createModel();

            if(return_val == FINGERPRINT_OK)
            {
                return_val = finger.storeModel(id_fingerprint);
                if(return_val == FINGERPRINT_OK)
                {
                    status = true;
                    break;
                }
            }

        }
        else
        {
            // Do nothing
        }
        time_out --;
    }
    return status;
}

unsigned int get_id()
{
    unsigned int id_fingerprint = 0;
    unsigned int time_out = 2000;
    String inputString = "";
    while(time_out > 0)
    {
        char key = keypad.getKey();
        if(mode == mode_setting_add_finger)
        {
            display_mode_add_fingerprint(mode_get_id,inputString);
        } 
        else if(mode == mode_setting_delete_finger)
        {
            display_mode_delete_fingerprint(mode_get_id, inputString, 0);
        }
        if(key >= '0' && key <= '9')
        {
            // id_fingerprint = id_fingerprint * 10 + (key - '0');
            inputString += key;
            if(debug_app)
            {
              Serial.print("id_fingerprint: ");
              Serial.println(id_fingerprint);
            }
        }
        else if(key == '#')
        {
            if (inputString.length() > 0) 
            {
                id_fingerprint = inputString.toInt();
                break;
            } 
            else 
            {
                if(debug_app)
                {
                  Serial.println("Invalid input");
                }
                inputString = "";
                break;
            }
        }
        else if(key == '*')
        {
            // break;
        }
        time_out-- ;
        // Serial.println(time_out);
    }
    return id_fingerprint;
}

void add_fingerprint_mode(){
    unsigned int id_fingerprint;
    bool return_val;
    while(1)
    {
        id_fingerprint = get_id();
        display_mode_add_fingerprint(mode_first_add_fingerprint, "0");
        if(id_fingerprint > 0){
            if( debug_app )
            {
              Serial.print("ID: ");
              Serial.println(id_fingerprint);
              Serial.println("First add");
            }
            return_val = enroll_fingerprint(id_fingerprint, mode_first_add_fingerprint);
            if(return_val)
            {
                // Serial.println("Second add");
                display_log(success_logo);
                notification(notif_success);
                display_mode_add_fingerprint(mode_second_add_fingerprint, "0");
                return_val = enroll_fingerprint(id_fingerprint, mode_second_add_fingerprint);
                if(return_val)
                {
                    // Serial.print("Success");
                    display_log(success_logo);
                    notification(notif_success);
                    display_mode_add_fingerprint(mode_success_add_fingerprint, "0");
                    saveFingerprintID(id_fingerprint);
                    delay(2000);
                    mode = mode_setting_none;
                    break;
                    /*
                    * Store id in the Preferences
                    */
                }
                else
                {
                    display_log(error_logo);
                    notification(notif_error);
                    display_mode_add_fingerprint(mode_success_add_fingerprint, "0");
                    // Serial.print("Error add second fingerprint");
                    mode = mode_setting_none;
                    delay(2000);
                    break;
                }
            }
            else
            {
                display_log(error_logo);
                notification(notif_error);
                display_mode_add_fingerprint(mode_error_add_fingerprint, "0");
                // Serial.print("Error add first fingerprint");
                mode = mode_setting_none;
                delay(2000);
                break;
            }
        } 
        else
        {
            display_log(error_logo);
            notification(notif_error);
            mode = mode_setting_none;
            break;
        }
    }
}


void display_mode_delete_fingerprint(unsigned char mode_display,String id, unsigned char mode_menu)
{
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 0);
    display.print("Del Finger");
    display.setTextSize(1);

    if( mode_display == mode_get_id){
        display.setTextSize(2);
        display.setCursor(25,35);
        display.print("ID: ");
        display.print(id);
    } 
    else if (mode_display == mode_confirm)
    {
        display.setCursor(25,25);
        display.print("Are you sure?");
        display.setCursor(45,35);
        display.print("Yes/No");
        display.setCursor(15,55);
        display.print("Yes");
        display.setCursor(90,55);
        display.print("No");

        if(mode_menu == 0){
            display.setCursor(5,55);
            display.print(">");
        } else if(mode_menu == 1){
            display.setCursor(80,55);
            display.print(">");
        }
    }
    else if (mode_display == mode_error)
    {
        display.setCursor(25,25);
        display.print("ID not found");
    }
    else if (mode_display == mode_success)
    {
        display.setCursor(25,25);
        display.print("Success");
    }
    display.display();
}

void delete_fingerprint_mode(){
    unsigned int id_fingerprint;
    char return_val = 1;
    unsigned char _mode = 0;
    unsigned int time_out = 2000;
    // Serial.println("delete fingerprint mode");
    id_fingerprint = get_id();
    if(id_fingerprint > 0){
        /*
        * Get confirm before deleting
        */
        while(time_out > 0)
        {
            char key = keypad.getKey();
            if(key == '*'){
                _mode = _mode == 1 ? 0 : 1;
            }
            else if(key == '#')
            {
                if(_mode == 0)
                {
                    /*Delete*/
                    return_val = finger.deleteModel(id_fingerprint);
                    if(debug_app)
                    {
                        Serial.print("Delete fingerprint: ");
                        Serial.println(return_val);
                    }
                    if(return_val == FINGERPRINT_OK)
                    {
                        deleteFingerprintID(id_fingerprint);
                        display_log(success_logo);
                        notification(notif_success);
                        /*
                        * Delete id in preference
                        */
                    }
                    else
                    {
                        // Serial.println("delete fingerprint mode error: ");
                        display_log(error_logo);
                        notification(notif_error);
                    }
                    mode = mode_setting_none;
                    delay(2000);
                    break;
                }
                else
                {
                    /*Cancel*/
                    mode = mode_setting_none;
                    break;
                }
            }
            display_mode_delete_fingerprint(mode_confirm, String(id_fingerprint), _mode);
            time_out--;
        }
        mode = mode_setting_none;
        
    }
    else
    {
        display_mode_delete_fingerprint(mode_error, String(id_fingerprint), _mode);
        mode = mode_setting_none;
    }
}

void show_list_finger()
{
    unsigned char _mode = 0;
    unsigned int count = 0;
    int list_id[100];
    int id;

    for (int i = 0; i < eeprom_size / sizeof(int); i++) {
        int address = i * sizeof(int);
        EEPROM.get(address, id); 

        if (id > 0 && id != 0xFFFF) { 
            list_id[count] = id; 
            count++;
        }
    }

    while(1)
    {
        char key = keypad.getKey();
        if(key == '#')
        {
            mode = mode_setting_none;
            break;
        }
        display_list_finger(_mode, list_id, count);
    }
}

void display_list_finger(unsigned char mode, int id_fingerprint[], unsigned int total_ids){

    int col[] = {5, 45, 85};
    int row[] = {15, 25, 35, 45};
    int count = 0;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(5,0);
    display.print("-- List finger --");

    // display.setCursor(5,15);
    // display.print("ID: ");
    // display.print(id_fingerprint[]);
    // display.setCursor(5,25);
    // display.setCursor(5,35);
    // display.setCursor(5,45);

    // display.setCursor(40,15);
    // display.setCursor(40,25);
    // display.setCursor(40,35);
    // display.setCursor(40,45);

    // display.setCursor(80,15);
    // display.setCursor(80,25);
    // display.setCursor(80,35);
    // display.setCursor(80,45);
    for (int i = 0; i< total_ids; i++)
    {
        int current_col = col[count % 3];
        int current_row = row[count / 3];
        display.setCursor(current_col, current_row);
        display.print("id:");
        display.print(id_fingerprint[i]);
        count++;
        if(count == 12)
        {
            break;
        }
    }

    if(mode == 0){
        display.setCursor(70,55);
        display.print(">");
    }
    display.setCursor(80,55);
    display.print("Exit");
    
    display.display();

}

void setting_clear_database()
{
    char unsigned _mode = 0;
    char return_val = 1;
    while(1)
    {
        char key = keypad.getKey();
        if(key == '*')
        {
            _mode = _mode == 1 ? 0 : 1;
        }
        else if(key == '#' && _mode == 0)
        {
            return_val = finger.emptyDatabase();
            if(return_val == FINGERPRINT_OK)
            {
                for (int i = 0; i < eeprom_size; i++) {
                    EEPROM.write(i, 0xFF);
                    EEPROM.commit();
                }
            }
            display_log(success_logo);
            notification(notif_success);
            mode = mode_setting_none;
            break;
        }
        else if(key == '#' && _mode == 1)
        {
            mode = mode_setting_none;
            break;
        }
        display_clear_database(_mode);
    }
}

void display_clear_database(unsigned char mode)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(5,0);
    display.print("-- Clear Database --");

    display.setCursor(25,25);
    display.print("Are you sure?");
    display.setCursor(45,35);
    display.print("Yes/No");
    display.setCursor(15,55);
    display.print("Yes");
    display.setCursor(90,55);
    display.print("No");

    if(mode == 0){
        display.setCursor(5,55);
        display.print(">");
    }
    else if(mode == 1){
        display.setCursor(80,55);
        display.print(">");
    }

    display.display();
}

void saveFingerprintID(int id) {
    int index = countSavedIDs();
    int address = index * sizeof(int); 
    if (address + sizeof(int) > eeprom_size) {
        Serial.println("EEPROM storage is full.");
        return;
    }
    EEPROM.put(address, id); 
    EEPROM.commit();        
    Serial.print("Fingerprint ID saved at index ");
    Serial.println(index);
}

int readFingerprintID(int index) {
    int address = index * sizeof(int); 
    int id;
    EEPROM.get(address, id); 
    Serial.print("Fingerprint ID at index ");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(id);
    return id;
}

int countSavedIDs() {
    int count = 0;
    for (int i = 0; i < eeprom_size / sizeof(int); i++) {
        int id;
        EEPROM.get(i * sizeof(int), id);
        if (id > 0 && id != 0xFFFF) {
            Serial.print("countSavedIDs: ");
            Serial.println(id);
            count++;
        }
    }
    return count;
}

void storeAllFingerprintIDs(int ids[], int &count) {
    count = 0; 
    int id;

    for (int i = 0; i < eeprom_size / sizeof(int); i++) {
        int address = i * sizeof(int);
        EEPROM.get(address, id); 

        if (id != 0xFFFF) { 
            ids[count] = id; 
            count++;
        }
    }
}

void deleteFingerprintID(int id_to_delete) {
    int index = findFingerprintIndex(id_to_delete); 
    if (index == -1) {
        if(debug_app)
        {
          Serial.print("ID ");
          Serial.print(id_to_delete);
          Serial.println(" not found.");
        }
        return;
    }

    int address = index * sizeof(int);
    int defaultValue = 0xFFFF;
    EEPROM.put(address, defaultValue); 
    EEPROM.commit();
    if(debug_app)
    {
        Serial.print("Fingerprint ID ");
        Serial.print(id_to_delete);
        Serial.print(" at index ");
        Serial.print(index);
        Serial.println(" has been deleted.");
    }
}

int findFingerprintIndex(int id_to_find) {
    for (int i = 0; i < eeprom_size / sizeof(int); i++) {
        int id;
        EEPROM.get(i * sizeof(int), id);
        if (id == id_to_find) {
            return i; 
        }
    }
    return -1;
}
