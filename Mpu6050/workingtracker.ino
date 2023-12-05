#define BLYNK_TEMPLATE_ID "TMPL6fj-tZuu4"
#define BLYNK_TEMPLATE_NAME "working tracked"
#define BLYNK_AUTH_TOKEN "p-tzz_Rs0oKR_gX8rXyMBgr9-ibhC9T9"
#define BLYNK_PRINT Serial

/**
 * Include libraries
*/

#include <WiFi.h>								
#include <WiFiClient.h>					
#include <BlynkSimpleEsp32.h>			
#include <Wire.h>								
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

/**
 * Define pinout interface
*/
#define buzz      16			
#define sensor1   19		  
#define sensor2   18		    
#define sensor3   5		  
#define button    17		  


// char ssid[] = "FPT-Tang 4";						
// char pass[] = "0359653222";				
char ssid[] = "working";						
char pass[] = "12345678";		
// 
/**
 * Global variables
*/
long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;

long gyroX, gyroY, gyroZ, time_run  = 0;
float rotX, rotY, rotZ;
float rotX1, rotY1, rotZ1;   // Bap tay
float rotX2, rotY2, rotZ2;   // Canh tay
float rotX3, rotY3, rotZ3;   // Co tay
unsigned char is_run = 0;

/**
* Nguong dung thang
* stand_Rank_rotX1: Bap tay
* stand_Rank_rotX3: Co tay
*/
float stand_Rank_rotX1 = 0.0; 
float stand_Rank_rotX3 = 0.0;

/**
* Nguong ngoi
* stand_Rank_rotX1: Bap tay
* stand_Rank_rotX3: Co tay
*/
float sit_rank_rotX1   = 0.0;
float sit_rank_rotX2   = 0.0;

/**
* Nguong nam ngang
* lay_rank_rotY1: Bap tay
* lay_rank_rotY3: Co tay
*/
float lay_rank_rotY1   = 0.0;
float lay_rank_rotY3   = 0.0;

BlynkTimer timer;
Adafruit_MPU6050 mpu;

WidgetLCD lcd(V3);

/*Function gui du lieu len blynk*/
void detectStatus()
{
  if((rotX3 > stand_Rank_rotX3) && (rotX1 > stand_Rank_rotX1))
  {
    lcd.print(3, 1, "Dung thang");  
  } else if ((rotY3 > lay_rank_rotY3)&&(rotY1 > lay_rank_rotY1))
  {
    lcd.print(3, 1, "Nam ngang ");
  } else if ((rotX1 > sit_rank_rotX1)&&(rotX2 > sit_rank_rotX2))
  {
    lcd.print(3, 1, "Ngoi      ");
  }
    
}

void notifications(){
  unsigned char i = 0;
  for(i = 0; i < 2; i++) {
    digitalWrite(buzz, HIGH); //Bat coi buzz
    delay(300);
    digitalWrite(buzz, LOW); //Tat coi buzz
    delay(300);
  }
}

void InitSensor(){
    for(unsigned char i = 0; i < 3; i++)
    {
      if(i == 0)
      {
        digitalWrite(sensor1, LOW);
        digitalWrite(sensor2, HIGH);
        digitalWrite(sensor3, HIGH);
      } 
      else if(i == 1)
      { 
        digitalWrite(sensor1, HIGH);
        digitalWrite(sensor2, LOW);
        digitalWrite(sensor3, HIGH);
      } 
      else 
      {
        digitalWrite(sensor1, HIGH);
        digitalWrite(sensor2, HIGH);
        digitalWrite(sensor3, LOW);
      }

      if (!mpu.begin()) {
          Serial.println("Failed to find MPU6050 chip");
          while (1) {
          delay(10);
          }
      }
        Serial.println("MPU6050 Found!");
        mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
        Serial.print("Accelerometer range set to: ");
        switch (mpu.getAccelerometerRange()) {
            case MPU6050_RANGE_2_G:
            Serial.println("+-2G");
            break;
            case MPU6050_RANGE_4_G:
            Serial.println("+-4G");
            break;
            case MPU6050_RANGE_8_G:
            Serial.println("+-8G");
            break;
            case MPU6050_RANGE_16_G:
            Serial.println("+-16G");
            break;
        }
        mpu.setGyroRange(MPU6050_RANGE_500_DEG);
        Serial.print("Gyro range set to: ");
        switch (mpu.getGyroRange()) {
            case MPU6050_RANGE_250_DEG:
            Serial.println("+- 250 deg/s");
            break;
            case MPU6050_RANGE_500_DEG:
            Serial.println("+- 500 deg/s");
            break;
            case MPU6050_RANGE_1000_DEG:
            Serial.println("+- 1000 deg/s");
            break;
            case MPU6050_RANGE_2000_DEG:
            Serial.println("+- 2000 deg/s");
            break;
        }
        mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
        Serial.print("Filter bandwidth set to: ");
        switch (mpu.getFilterBandwidth()) {
            case MPU6050_BAND_260_HZ:
            Serial.println("260 Hz");
            break;
            case MPU6050_BAND_184_HZ:
            Serial.println("184 Hz");
            break;
            case MPU6050_BAND_94_HZ:
            Serial.println("94 Hz");
            break;
            case MPU6050_BAND_44_HZ:
            Serial.println("44 Hz");
            break;
            case MPU6050_BAND_21_HZ:
            Serial.println("21 Hz");
            break;
            case MPU6050_BAND_10_HZ:
            Serial.println("10 Hz");
            break;
            case MPU6050_BAND_5_HZ:
            Serial.println("5 Hz");
            break;
        }
  }
}


void setup() {
  Serial.begin(115200);
  /* Cai dat cac chan sensor va chan buzz la output*/
  pinMode(sensor1,  OUTPUT);
  pinMode(sensor2,  OUTPUT);
  pinMode(sensor3,  OUTPUT);
  pinMode(buzz,     OUTPUT);
  pinMode(button,   INPUT_PULLUP);	

  InitSensor();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  lcd.clear(); //Use it to clear the LCD Widget
  lcd.print(3, 0, "Trang thai"); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
  lcd.print(3, 1, "");      
  notifications();
  // timer.setInterval(500L, sendDataToBlynk);  

  // time_run = millis();
}

/**
   Ham thuc thi chuong trinh
   Khoi tao ket noi toi blynk
   Doc gia tri nut nhan
   --> Neu nut nhan duoc nhan
   -----> Chay chuong trinh doc cam bien
   ---------> Neu nut nhan duoc nhan lan nua --> Tat chuong trinh doc cam bien
*/

void loop() {
  Blynk.run();
  Blynk.run();
  timer.run(); // Initiates BlynkTimer
  if ((millis() - time_run > 0) && (millis() - time_run < 2000)) { //500ms doc cam bien 1
    getSensor(1);
    GetData();
  } else if ((millis() - time_run > 2000) && (millis() - time_run < 3000)) { //500ms doc cam bien 2
    getSensor(2);
    GetData();
  } else if ((millis() - time_run > 3000) && (millis() - time_run < 4000)) { //500ms doc cam bien 3
    getSensor(3);
    GetData();
  } else if (millis() - time_run > 4000) {
    time_run = millis();
  }
  detectStatus();
}
/* Doc cam bien */
void GetData() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  rotX = a.acceleration.x;
  rotY = a.acceleration.y;
  rotZ = a.acceleration.z;
}

void printData(unsigned char sensor, float x, float y,float z ){
  Serial.print("Sensor: ");
  Serial.println(sensor);
  Serial.print("Acceleration X: ");
  Serial.print(x);
  Serial.print(", Y: ");
  Serial.print(y);
  Serial.print(", Z: ");
  Serial.print(z);
  Serial.println(" m/s^2");
  Serial.println("");
}

/* Chon sensor can doc*/

void getSensor(unsigned char sensor) {
  if (sensor == 1) { //Chon sensor1
    digitalWrite(sensor1, LOW);
    digitalWrite(sensor2, HIGH);
    digitalWrite(sensor3, HIGH);
    rotX1 = rotX;
    rotY1 = rotY;
    rotZ1 = rotZ;
    printData(1,rotX1,rotY1,rotZ1);
    Blynk.virtualWrite(V1, rotX1);
    Blynk.virtualWrite(V2, rotY1);
    // Blynk.virtualWrite(V3, rotZ1);
  } else if (sensor == 2) { //Chon sensor2
    digitalWrite(sensor1, HIGH);
    digitalWrite(sensor2, LOW);
    digitalWrite(sensor3, HIGH);
    rotX2 = rotX;
    rotY2 = rotY;
    rotZ2 = rotZ;
    printData(2,rotX2,rotY2,rotZ2);
    Blynk.virtualWrite(V4, rotX2);
    Blynk.virtualWrite(V5, rotY2);
    // Blynk.virtualWrite(V6, rotZ2);
  } else { //Chon sensor3
    digitalWrite(sensor1, HIGH);
    digitalWrite(sensor2, HIGH);
    digitalWrite(sensor3, LOW);
    rotX3 = rotX;
    rotY3 = rotY;
    rotZ3 = rotZ;
    printData(3,rotX3,rotY3,rotZ3);
    Blynk.virtualWrite(V7, rotX3);
    Blynk.virtualWrite(V8, rotY3);
    // Blynk.virtualWrite(V9, rotZ3);
  } 
}

