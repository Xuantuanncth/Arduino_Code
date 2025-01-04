Collecting workspace information

To structure the documentation for the 

finger_printer.ino

 file, you can follow this outline:

1. **Project Overview**
   - Brief description of the project and its purpose.

2. **Hardware Components**
   - List of hardware components used in the project.

3. **Libraries Included**
   - List of libraries included and their purpose.

4. **Pin Definitions**
   - Description of pin definitions and their usage.

5. **Display Configuration**
   - Configuration details for the OLED display.

6. **Notification Types**
   - Explanation of notification types.

7. **Mode Definitions**
   - Description of different modes used in the project.

8. **Global Variables**
   - List and description of global variables.

9. **Functions**
   - Detailed description of each function, including its purpose, parameters, and return values.

10. **Setup and Loop**
    - Explanation of the 

setup

 and 

loop

 functions.

Here is an example of how you can structure the documentation:

---

# FingerPrinter with ESP32

## Project Overview
This project implements a fingerprint-based attendance system using an ESP32 microcontroller. It uses various libraries to interface with hardware components such as an OLED display, a fingerprint sensor, a keypad, and an RTC module.

## Hardware Components
- ESP32 microcontroller
- OLED display (128x64)
- Fingerprint sensor
- Keypad (4x3)
- RTC module (DS1307)
- Buzzer

## Libraries Included
- `SPI.h`: SPI communication library.
- 

Wire.h

: I2C communication library.
- `Adafruit_GFX.h`: Graphics library for the OLED display.
- 

Adafruit_SSD1306.h

: OLED display driver library.
- 

Keypad.h

: Keypad input library.
- 

Adafruit_Fingerprint.h

: Fingerprint sensor library.
- 

HardwareSerial.h

: Serial communication library.
- `RTClib.h`: RTC library.
- 

EEPROM.h

: EEPROM library for storing fingerprint IDs.

## Pin Definitions
- 

buzz_pin

: Pin 4 for the buzzer.

## Display Configuration
- 

screen_width

: 128 pixels
- 

screen_height

: 64 pixels
- 

oled_reset

: -1 (no reset pin)
- 

screen_address

: 0x3C (I2C address)

## Notification Types
- 

notif_success

: 1
- 

notif_error

: 2

## Mode Definitions
- 

mode_first_add_fingerprint

: 1
- 

mode_second_add_fingerprint

: 2
- 

mode_success_add_fingerprint

: 3
- 

mode_error_add_fingerprint

: 4
- 

mode_get_id

: 5
- 

mode_confirm

: 6
- 

mode_success

: 7
- 

mode_error

: 8

## Global Variables
- 

Adafruit_SSD1306 display

: OLED display object.
- 

HardwareSerial mySerial

: Serial communication object for the fingerprint sensor.
- 

Adafruit_Fingerprint finger

: Fingerprint sensor object.
- 

RTC_DS1307 rtc

: RTC object.
- 

Keypad keypad

: Keypad object for user input.
- 

unsigned char mode

: Current mode of the system.
- 

unsigned char id_checking

: ID being checked.
- 

unsigned char in_hour, in_minute, out_hour, out_minute

: Time settings for the working day.
- 

unsigned char current_hour, current_min, current_sec

: Current time.
- `const char* status[]`: Status messages.
- 

char current_date[16]

: Current date string.
- 

char current_time[10]

: Current time string.
- 

char message[40]

: Message buffer.
- 

bool debug_app

: Debug flag.

## Functions

### 

void setup()


Initializes the serial communication, fingerprint sensor, EEPROM, OLED display, and RTC module.

### 

void loop()


Main loop that handles different modes and user inputs.

### 

void get_time_from_rtd()


Fetches the current time from the RTC module.

### 

void send_data_to_application()


Sends attendance data to the application.

### 

String read_data_from_application()


Reads data from the application.

### 

void setting_menu_mode()


Handles the setting menu mode.

### 

void display_menu_mode(unsigned char page, unsigned char mode_menu)


Displays the setting menu on the OLED display.

### 

void display_setting_time(unsigned char mode_display)


Displays the time setting screen on the OLED display.

### 

unsigned char input_number(char key, unsigned char current_number, String *inputNumber)


Handles numeric input from the keypad.

### 

void setting_time_mode()


Handles the time setting mode.

### 

void display_welcome(unsigned char mode)


Displays the welcome screen on the OLED display.

### 

void display_name_employee(String name)


Displays the employee name on the OLED display.

### 

void display_log(const uint8_t *logo)


Displays a logo on the OLED display.

### 

void display_mode_add_fingerprint(unsigned char mode_display, String id)


Displays the add fingerprint mode on the OLED display.

### 

void notification(unsigned char notification_type)


Handles buzzer notifications.

### 

unsigned char detect_fingerprint()


Detects a fingerprint and returns the detection state.

### 

void check_fingerprint()


Checks for a fingerprint and handles the result.

### 

bool enroll_fingerprint(unsigned char id_fingerprint, unsigned char mode_enroll)


Enrolls a fingerprint.

### 

unsigned int get_id()


Gets an ID from the user.

### 

void add_fingerprint_mode()


Handles the add fingerprint mode.

### 

void display_mode_delete_fingerprint(unsigned char mode_display, String id, unsigned char mode_menu)


Displays the delete fingerprint mode on the OLED display.

### 

void delete_fingerprint_mode()


Handles the delete fingerprint mode.

### 

void show_list_finger()


Displays the list of stored fingerprints.

### 

void display_list_finger(unsigned char mode, int id_fingerprint[], unsigned int total_ids)


Displays the list of fingerprints on the OLED display.

### 

void setting_clear_database()


Handles the clear database mode.

### 

void display_clear_database(unsigned char mode)


Displays the clear database confirmation screen on the OLED display.

### 

void saveFingerprintID(int id)


Saves a fingerprint ID to EEPROM.

### 

int readFingerprintID(int index)


Reads a fingerprint ID from EEPROM.

### 

int countSavedIDs()


Counts the number of saved fingerprint IDs in EEPROM.

### 

void storeAllFingerprintIDs(int ids[], int &count)


Stores all fingerprint IDs from EEPROM into an array.

### 

void deleteFingerprintID(int id_to_delete)


Deletes a fingerprint ID from EEPROM.

### 

int findFingerprintIndex(int id_to_find)


Finds the index of a fingerprint ID in EEPROM.

---

This structure provides a comprehensive overview of the project, its components, and the functionality of the code.