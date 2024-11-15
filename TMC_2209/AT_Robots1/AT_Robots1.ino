// ---------------- Option 1 - SỬ DỤNG ĐỘNG CƠ NEMA17 LÀM ĐỘNG CƠ CẤP BÓNG ------------------
// ---------------- Option 2 - SỬ DỤNG ĐỘNG CƠ DC 12V LÀM ĐỘNG CƠ CẤP BÓNG ------------------

// !!!!!   Công việc đã được thử nghiệm trên các phiên bản thư viện này   !!!!!
// Arduino IDE 1.8.16
// ESPUI  2.0.2
// ESP32Servo 0.9.0
// ESPAsyncWebServer 1.2.3
// ArduinoJson  6.18.4
// AsyncTCP 1.1.1


// Chúng tôi kết nối thư viện servo, đối với ESP32 chúng tôi cần <ESP32Servo.h>
#include <ESP32Servo.h>
//#include <SPI.h>
#include <AccelStepper.h>   // khai báo thư viện hỗ trợ động cơ bước 
// Thư viện động cơ bước
//#include <GyverStepper.h> 

// declare Servo objects
Servo Motor_1;   // động cơ phía trên bên phải
Servo Motor_2;   // động cơ trên cùng bên trái
Servo Motor_3;   // động cơ phía dưới
Servo Servo_L_R; // servo trái- phải
Servo Servo_U_D; // servo lên - xuống

//--------------------Option 2-----------------------------
  // Động cơ servo_BallS; // động cơ cho bóng ăn
//----------------------------------------------------------

// Chúng tôi kết nối thư viện EEPROM, chúng tôi sẽ lưu các quả bóng vào bộ nhớ
#include <EEPROM.h>
#define EEPROM_SIZE 2048  // kích thước tính bằng byte

// Chúng tôi kết nối thư viện FASTLED để biểu thị chuyển động quay của động cơ
#include <FastLED.h>
// Sử dụng led WS2812b
#define NUM_LEDS 24
CRGB leds[NUM_LEDS];


//------------------------------------------------------------------------

// Kết nối
static const int Motor_1_Pin = 18; // ESC 1  điều khiển động cơ phía trên bên phải 1 chân D18
static const int Motor_2_Pin = 19; // ESC 2 điều khiển động cơ trên cùng bên trái 2 chân D19
static const int Motor_3_Pin = 21; // ESC 3 điều khiển động cơ phía dưới 3 chân D21
static const int Servo_L_R_Pin = 27; // SERRVO TRAI - PHAI điều khiển servo chân trái-phải D27
static const int Servo_U_D_Pin = 26; // SERVO LEN XUONG chân lên - xuống điều khiển servo D26
const int Motor_Mixing_Balls_Pin = 25; // ĐỘNG CƠ M5 điều khiển mô tơ khuấy bóng M5 chân D25
const int Motor_BallS_Pin = 16; // ĐỘNG CƠ M4  điều khiển Động cơ nạp bóng M4 chân D16
int Led_Infrared = 13; // CẢM BIẾN HỒNG NGOẠI Chân thu cảm biến diode IR D13
#define DATA_PIN 5 // LED RGB Led hiện thị diodes WS2812b pin D5 

//--------------------------!!!!!!!!!!!!!!!!!!!!!!!!-------------------------------------

// các biến này cần được điều chỉnh (cơ học và ESC là khác nhau)
// ------------------------- ĐỘNG CƠ 1 ------------------------------
int Motor_1_min_forv = 1507; // động cơ 1 - tốc độ chuyển tiếp tối thiểu
int Motor_1_max_forv = 1804; // động cơ 1 - tốc độ chuyển tiếp tối đa
int Motor_1_stop = 1470;     // động cơ 1 - dừng
int Motor_1_min_back = 1442; // động cơ 1 - tốc độ lùi tối thiểu
int Motor_1_max_back = 1145; // động cơ 1 - tốc độ lùi tối đa
// -------------------------- ĐỘNG CƠ 2 --------------------------------
int Motor_2_min_forv = 1507; // động cơ 2 - tốc độ chuyển tiếp tối thiểu
int Motor_2_max_forv = 1804; // động cơ 2 - tốc độ chuyển tiếp tối đa
int Motor_2_stop = 1470;     // động cơ 2 - dừng
int Motor_2_min_back = 1442; // động cơ 2 - tốc độ lùi tối thiểu
int Motor_2_max_back = 1145; // động cơ 2 - tốc độ lùi tối đa
// -------------------------- ĐỘNG CƠ 3 -------------------------------
int Motor_3_min_forv = 1507; // động cơ 3 - tốc độ chuyển tiếp tối thiểu
int Motor_3_max_forv = 1804; // động cơ 3 - tốc độ chuyển tiếp tối đa
int Motor_3_stop = 1470;     // động cơ 3 - dừng
int Motor_3_min_back = 1442; // động cơ 3 - tốc độ lùi tối thiểu
int Motor_3_max_back = 1145; // động cơ 3 - tốc độ lùi tối đa
// Servo_TRÁI - PHẢI
int Servo_L_R_min = 0;  // servo trái-phải - góc quay tối thiểu
int Servo_L_R_max = 120; // servo trái-phải - góc quay tối đa
int Servo_L_R_middle = 60; // servo trái-phải - giữa
// --------------------- Servo LÊN - XUỐNG -----------------------------------
int Servo_U_D_min = 5;     // servo lên xuống - góc nâng tối đa
int Servo_U_D_max = 175;    // servo lên xuống - góc hạ xuống tối thiểu
int Servo_U_D_middle = 90; // servo lên xuống - giữa
// ----------------------- ĐỘNG CƠ TRỘN BÓNG -----------------------------------
int Motor_Mixing_Balls_forv = 120; // động cơ khuấy bóng quay về phía trước
int Motor_Mixing_Balls_stop = 0; // dừng động cơ khuấy bóng


// -------------------ĐỘNG CƠ CẤP BÓNG---------------------------------------------------

// BỎ DẤU # NẾU CHỌN PHIÊN BẢN CẤP BÓNG BẰNG ĐỘNG CƠ STEP NEMA17
//-------------------- TÙY CHỌN 1 - SỬ DỤNG ĐỘNG CƠ STEP NEMA17 -------------------------
   // Nema 17 cao độ 1,8 độ. 1 vòng quay 200 bước mỗi giây. Ở chế độ bước 1/16 3200 bước/giây, bánh răng 1/3
  int Motor_Balls_min = 3200;   // Động cơ bước cấp bóng - 30 quả bóng mỗi phút
  int Motor_Balls_max = 12800;   // Động cơ bước bóng - 120 quả bóng mỗi phút
  int Motor_Balls_stop = 0;  // Động cơ bước cấp bóng - dừng
  #define motorDir 33
  AccelStepper stepper(1, Motor_BallS_Pin, motorDir); //khởi tạo đối tượng động cơ bước NEMA 17
  int stepper_delay_time = 500; // Thời gian đợi bắn xong để nạp là 100ms
  int stepper_time = 0; // Thời gian đợi lấy bóng
  float set_speed_Balls = 0; // tốc độ lấy bóng
  int Stepper_MS = 16; // số micro step => cần 16 microstep để được 1 step
  boolean Stepper_Get_Ball_Flag_Done = false;
  boolean Stepper_Start_Get_Ball = false;
  int stepper_counter = 0;
//-------------------- TÙY CHỌN 2 - SỬ DỤNG ĐỘNG CƠ GIẢM TỐC -------------------------------------
// BỎ DẤU # NẾU CHỌN PHIÊN BẢN CẤP BÓNG BẰNG ĐỘNG CƠ GIẢM TỐC
    // động cơ cấp bóng, động cơ chuyển mạch điều khiển bằng bộ truyền động bằng bánh răng
 // int Motor_Balls_min = 70;  // Động cơ nạp bóng - 30 quả bóng mỗi phút
 // int Motor_Balls_max = 255; // Động cơ nạp bóng - 120 quả bóng mỗi phút
 // int Motor_Balls_stop = 0;  // động cơ nạp bóng - dừng
//------------------------------------------------------------------------

// các biến trong chương trình trên trang web

// chung cho tất cả các TAB
boolean start = false; // true - start, false - stop
int Motor_Balls_Speed = 0; // Tốc độ của động cơ nạp bóng

// các biến trên tab "BALLS"
int Ball_Number = 1; 
int B_STR; 
int M1_STR;
int M2_STR;
int M3_STR;
int UUD_STR;
int ULR_STR;
int R_STR;
int P_STR;
// Khối biến ESPUI trên tab "BALLS"
uint16_t select4 = 1;
int B_N; 
int M1_N;
int M2_N;
int M3_N;
int UUD_N;
int ULR_N;
int R_N;
int P_N;

// các biến trên tab "MMR"
int Mem_Cell = 1; // vị trí bộ nhớ của chương trình
boolean load_Memory = false;
boolean save_Memory = false;
// Khối biến ESPUI trên tab "MMR"
uint16_t select1 = 1;
uint16_t text_cell;
uint16_t button_Load;
uint16_t button_Save;

// các biến trên tab "MMR 2"
int Select_Ball = 1; // Bóng được chọn để ghi nhớ
int Select_Memory = 1; // bộ nhớ bóng
boolean Load_Ball = false;
boolean Save_Ball = false;
// Khối biến ESPUI trên tab "MMR 2"
uint16_t select2 = 1;
uint16_t select3 = 1;
uint16_t text_cell_2;
uint16_t button_Load_2;
uint16_t button_Save_2;

// các biến trên tab "CHƯƠNG TRÌNH"
boolean STOP_Mixing = false; // true - start, false - stop trộn bóng
// Khối biến ESPUI trên tab "CHƯƠNG TRÌNH"
uint16_t PROG;
uint16_t text_programm;

// thông số bóng
 // 1 ball
int ball_N1 = 1; // number of 1st balls
int Motor_1_speed_N1 = 0; //tốc độ động cơ 1 cho quả bóng thứ 1
int Motor_2_speed_N1 = 0; //tốc độ động cơ 1 cho quả bóng thứ 2
int Motor_3_speed_N1 = 0; //tốc độ động cơ 1 cho quả bóng thứ 3
int ugol_U_D_N1 = 50; // góc khởi hành trên-dưới của quả bóng thứ nhất
int ugol_L_R_N1 = 50; // góc khởi hành trái-phải cho quả bóng đầu tiên
int random_N1 = 0; // kích thước của phép cộng ngẫu nhiên vào góc thoát trái-phải của quả bóng thứ nhất
int pausa_N1 = 0; // tạm dừng kích thước trước quả bóng đầu tiên
 // 2 ball
int ball_N2 = 0; // number of 2nd balls (0 - no)
int Motor_1_speed_N2 = 0; // speed of motor 1 for 2 balls
int Motor_2_speed_N2 = 0; // speed of motor 2 for 2 balls
int Motor_3_speed_N2 = 0; //motor speed 3 for 2 balls
int ugol_U_D_N2 = 50; // departure angle higher-lower for 2 balls
int ugol_L_R_N2 = 50; // departure angle left-right for 2 balls
int random_N2 = 0; // the size of the random addition to the left-right departure angle for 2 balls
int pausa_N2 = 0; // pause size before 2nd ball
 // 3 ball
int ball_N3 = 0; // number of 3rd balls (0 - no)
int Motor_1_speed_N3 = 0; //speed of motor 1 for 3rd balls
int Motor_2_speed_N3 = 0; //speed of motor 2 for 3rd balls
int Motor_3_speed_N3 = 0; // speed of motor 3 for 3 balls
int ugol_U_D_N3 = 50; // departure angle higher-lower for 3 balls
int ugol_L_R_N3 = 50; // departure angle left-right for 3 balls
int random_N3 = 0; // the size of the random addition to the left-right departure angle for the 3rd balls
int pausa_N3 = 0; // pause size before 3rd ball
// 4 ball
int ball_N4 = 0; // number of 4th balls (0 - no)
int Motor_1_speed_N4 = 0; // speed of motor 1 for 4 balls
int Motor_2_speed_N4 = 0; // speed of motor 2 for 4 balls
int Motor_3_speed_N4 = 0; // speed of motor 3 for 4 balls
int ugol_U_D_N4 = 50; // departure angle higher-lower for 4 balls
int ugol_L_R_N4 = 50; // departure angle left-right for 4 balls
int random_N4 = 0; // the size of the random addition to the left-right departure angle for the 4th balls
int pausa_N4 = 0; // pause size before 4th ball
  // 5 ball 
int ball_N5 = 0; // number of 5th balls (0 - no)
int Motor_1_speed_N5 = 0; //speed of motor 1 for 5th balls
int Motor_2_speed_N5 = 0; //speed of motor 2 for 5th balls
int Motor_3_speed_N5 = 0; //speed of motor 3 for 5th balls
int ugol_U_D_N5 = 50; // departure angle higher-lower for the 5th balls
int ugol_L_R_N5 = 50; // departure angle left-right for 5th balls
int random_N5 = 0; // the size of the random addition to the left-right departure angle for the 5th balls
int pausa_N5 = 0; // pause size before 5th ball
 // 6 ball  
int ball_N6 = 0; // number of 6th balls (0 - no)
int Motor_1_speed_N6 = 0; // speed of motor 1 for 6 balls
int Motor_2_speed_N6 = 0; //motor speed 2 for 6 balls
int Motor_3_speed_N6 = 0; //speed of motor 3 for 6th balls
int ugol_U_D_N6 = 50; // departure angle higher-lower for 6 balls
int ugol_L_R_N6 = 50; // departure angle left-right for 6 balls
int random_N6 = 0; // the size of the random addition to the left-right departure angle for the 6th balls
int pausa_N6 = 0; // pause size before 6th ball
 // 7 ball  
int ball_N7 = 0; // number of 7th balls (0 - no)
int Motor_1_speed_N7 = 0; //speed of motor 1 for 7th balls
int Motor_2_speed_N7 = 0; //speed of motor 2 for 7th balls
int Motor_3_speed_N7 = 0; //speed of motor 3 for 7th balls
int ugol_U_D_N7 = 50; // departure angle higher-lower for the 7th balls
int ugol_L_R_N7 = 50; // departure angle left-right for the 7th balls
int random_N7 = 0; // the size of the random addition to the left-right departure angle for the 7th balls
int pausa_N7 = 0; // pause size before 7th ball
 // 8 ball  
int ball_N8 = 0; // number of 8 balls (0 - no)
int Motor_1_speed_N8 = 0; //motor speed 1 for 8 balls
int Motor_2_speed_N8 = 0; //motor speed 2 for 8 balls
int Motor_3_speed_N8 = 0; //motor speed 3 for 8th balls
int ugol_U_D_N8 = 50; // departure angle higher-lower for 8 balls
int ugol_L_R_N8 = 50; // departure angle left-right for 8 balls
int random_N8 = 0; // the size of the random addition to the left-right departure angle for the 8th balls
int pausa_N8 = 0; // pause size before 8th ball
 // 9 ball  
int ball_N9 = 0; // number of 9th balls (0 - no)
int Motor_1_speed_N9 = 0; //speed of motor 1 for 9th balls
int Motor_2_speed_N9 = 0; //speed of motor 2 at 9th balls
int Motor_3_speed_N9 = 0; //speed of motor 3 for 9th balls
int ugol_U_D_N9 = 50; // departure angle higher-lower for the 9th balls
int ugol_L_R_N9 = 50; // departure angle left-right for the 9th balls
int random_N9 = 0; // the size of the random addition to the left-right departure angle for the 9th balls
int pausa_N9 = 0; // pause size before 9th ball
 // 10 ball
int ball_N10 = 0; // number of 10th balls
int Motor_1_speed_N10 = 0; //motor speed 1 for 10th balls
int Motor_2_speed_N10 = 0; //motor speed 2 at 10th balls
int Motor_3_speed_N10 = 0; //motor speed 2 at 10th balls
int ugol_U_D_N10 = 50; // departure angle higher-lower for 10th balls
int ugol_L_R_N10 = 50; // departure angle left-right for 10th balls
int random_N10 = 0; // the size of the random addition to the left-right departure angle for the 10th balls
int pausa_N10 = 0; // pause size before 10th ball


// biến phụ trợ
int ball_N = 1; // số bóng (1 trên 6)
int ball_S = 1; //tổng số quả bóng
int nBall = 0; // số lượng bóng để xoay servo trái-phải
int sBall = 0; // số quả bóng để tính tốc độ
boolean LedInfrared_Flag = 0; // cờ cho máy đếm bóng
boolean Start_Flag = 0; // cờ để đặt lại bộ đếm bóng khi bắt đầu
boolean Pausa_Flag = false; // cờ để dừng giao bóng trong thời gian tạm dừng
boolean Pausa_Flag_N = false; // cờ để sửa thời gian bắt đầu dừng
boolean Flag_balls_change = false; // cờ đổi bóng sang tạm dừng để nó chỉ tạm dừng 1 lần
unsigned long time_Pausa = 0; // biến để tính thời gian dừng
unsigned long time_Schetchik = 0; // biến để tính thời gian dừng của bộ đếm bóng
boolean Flag_balls_Schetchik = false; // cờ để tạm dừng việc đếm bóng
boolean random_Flag = true; // để thiết lập một sự bổ sung ngẫu nhiên cho góc khởi hành trái-phải
boolean random_Flag_Balls = true; // để đặt một quả bóng ngẫu nhiên từ số đã cho
boolean random_Balls = false; // để đặt một quả bóng ngẫu nhiên từ số đã cho
String Programm_text; // mô tả chương trình trên "Programm Parameters" tab
int N_Ppog_back = -1; // số chương trình cũ
boolean load_memory_Flag = false; // cờ để chặn việc tải một loạt bóng
int Mem_Cell_Back = -1; // số ô nhớ của dòng cũ
boolean save_memory_Flag = false; // cờ chặn việc ghi một loạt bóng
boolean Flag_Load_ball = false; // cờ chặn tải bóng
boolean Flag_Save_Ball = false; // cờ khóa kỷ lục bóng
int Select_Memory_back = -1; // số bộ nhớ bóng cũ
int Ball_Number_back = -1; // số bóng cũ
int Motor_Balls_Speed_back = 0; // tốc độ động cơ thức ăn bóng cũ

// nhiệm vụ trung gian
 // tốc độ -100 - lùi, 0 - dừng, 100 - tiến
 // góc khởi hành trên-dưới 0 - dưới, 50 - giữa, 100 - trên
 // góc khởi hành trái-phải 0 - trái, 50 - giữa, 100 - phải
int Motor_1_speed = 0; // tốc độ động cơ tham chiếu trung gian 1
int Motor_2_speed = 0; // tốc độ động cơ tham chiếu trung gian 2
int Motor_3_speed = 0; // tốc độ động cơ tham chiếu trung gian 3
int ugol_UD = 50; // góc khởi hành nhiệm vụ trung gian trên-dưới
int ugol_LR = 50; // góc khởi hành nhiệm vụ trung gian trái-phải
int random_Ugol = 0; // nhiệm vụ trung gian của việc bổ sung ngẫu nhiên vào góc khởi hành trái-phải
int pausa_PRM = 0; // nhiệm vụ tạm dừng sau khi bóng bay ra

// công việc thực tế
int SPD_M1 = 0; // tốc độ động cơ 1
int SPD_M2 = 0; // tốc độ động cơ 2
int SPD_M3 = 0; // tốc độ động cơ 3
int UGL_UD = 50; // góc khởi hành trên-dưới
int UGL_LR = 50; // góc khởi hành trái-phải
int UGL_random = 0; // phép cộng ngẫu nhiên vào góc khởi hành trái-phải
int UGL_random_R = 0; // ngoài góc khởi hành trái-phải

//------------------------------------------------------------------------

// ở đây được tạo mạng WI_FI "A&T ROBOTS" và trang có IP (192, 168, 4, 1)
#include <DNSServer.h>
#include <ESPUI.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

#include <WiFi.h>

const char *ssid = "A&T ROBOTS"; // ESPUI
const char *password = ""; // tại đây bạn có thể đặt mật khẩu WI-FI

const char *hostname = "espui";

//---------------------------------------------------------------------------------
void switchExample(Control *sender, int value) {
    if (value == HIGH) {
        Serial.println("Switch turned ON");
        // Xử lý logic khi bật công tắc
    } else {
        Serial.println("Switch turned OFF");
        // Xử lý logic khi tắt công tắc
    }
}

void slider(Control *sender, int value) {
    Serial.print("Slider value: ");
    Serial.println(value);
    // Xử lý logic khi giá trị của slider thay đổi
}


void setup()   { 
  Serial.begin(115200);               
 // mọi thứ đều có ở đây cho WI-FI
    ESPUI.setVerbosity(Verbosity::VerboseJSON);

    WiFi.setHostname(hostname);
  
  // cố gắng kết nối với mạng hiện có
  WiFi.begin(ssid, password);
  {
    uint8_t timeout = 10;
    // Đợi kết nối, thời gian chờ 5 giây
    do {
      delay(500);
      timeout--;
    } while (timeout && WiFi.status() != WL_CONNECTED);

    // not connected -> tạo điểm phát sóng
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      WiFi.softAP(ssid);
      timeout = 5;
      do {
        delay(500);
        timeout--;
      } while (timeout);
    }
  }

  dnsServer.start(DNS_PORT, "*", apIP); 

  // đây là các khối ESPUI tạo thành trang Web
  uint16_t tab1 = ESPUI.addControl( ControlType::Tab, "BALLS", "BALLS" );   // id 01  
  uint16_t tab2 = ESPUI.addControl( ControlType::Tab, "MMR", "MMR" );         // id 02
  uint16_t tab3 = ESPUI.addControl( ControlType::Tab, "MMR 2", "MMR 2" );     // id 03
  uint16_t tab4 = ESPUI.addControl( ControlType::Tab, "PROGR", "PROGR" ); // id 04
  // các khối ESPUI phổ biến
  ESPUI.switcher("BẮT ĐẦU", &switchExample, ControlColor::None, false); // id 5
  ESPUI.slider("TỐC ĐỘ BÓNG", &slider, ControlColor::Turquoise, 0); // id 6
  // tab ở đây BÓNG
  select4 = ESPUI.addControl( ControlType::Select, "Select Ball :", "", ControlColor::Alizarin, tab1, &selectExample );  // id 9
  ESPUI.addControl( ControlType::Option, "BALL 1", "1", ControlColor::Alizarin, select4 );
  ESPUI.addControl( ControlType::Option, "BALL 2", "2", ControlColor::Alizarin, select4 );
  ESPUI.addControl( ControlType::Option, "BALL 3", "3", ControlColor::Alizarin, select4 );
  ESPUI.addControl( ControlType::Option, "BALL 4", "4", ControlColor::Alizarin, select4 );
  ESPUI.addControl( ControlType::Option, "BALL 5", "5", ControlColor::Alizarin, select4 );
  ESPUI.addControl( ControlType::Option, "BALL 6", "6", ControlColor::Alizarin, select4 );
  ESPUI.addControl( ControlType::Option, "BALL 7", "7", ControlColor::Alizarin, select4 );
  ESPUI.addControl( ControlType::Option, "BALL 8", "8", ControlColor::Alizarin, select4 );
  ESPUI.addControl( ControlType::Option, "BALL 9", "9", ControlColor::Alizarin, select4 );
  ESPUI.addControl( ControlType::Option, "BALL 10", "10", ControlColor::Alizarin, select4 );
  B_N = ESPUI.addControl( ControlType::Number, "BALL :", "1", ControlColor::Emerald, tab1, &numberCall ); // id 20
  M1_N = ESPUI.addControl( ControlType::Number, "MOTOR 1", "0", ControlColor::Peterriver, tab1, &numberCall ); // id 21
  M2_N = ESPUI.addControl( ControlType::Number, "MOTOR 2", "0", ControlColor::Peterriver, tab1, &numberCall ); // id 22
  M3_N = ESPUI.addControl( ControlType::Number, "MOTOR 3", "0", ControlColor::Peterriver, tab1, &numberCall ); // id 23
  UUD_N = ESPUI.addControl( ControlType::Slider, "LÊN - XUỐNG", "50", ControlColor::Carrot, tab1, &slider ); // id 24
  ULR_N = ESPUI.addControl( ControlType::Slider, "TRÁI - PHẢI", "50", ControlColor::Carrot, tab1, &slider ); // id 25
  R_N = ESPUI.addControl( ControlType::Number, "RANDOM", "0", ControlColor::Turquoise, tab1, &numberCall ); // id 26
  P_N = ESPUI.addControl( ControlType::Number, "TẠM DỪNG", "0", ControlColor::None, tab1, &numberCall ); // id 27
  // here is the MMR tab 
  select1 = ESPUI.addControl( ControlType::Select, "Memory Cell :", "", ControlColor::Alizarin, tab2, &selectExample );  // id 28
  ESPUI.addControl( ControlType::Option, "Cell 1", "1", ControlColor::Alizarin, select1 );
  ESPUI.addControl( ControlType::Option, "Cell 2", "2", ControlColor::Alizarin, select1 );
  ESPUI.addControl( ControlType::Option, "Cell 3", "3", ControlColor::Alizarin, select1 );
  ESPUI.addControl( ControlType::Option, "Cell 4", "4", ControlColor::Alizarin, select1 );
  ESPUI.addControl( ControlType::Option, "Cell 5", "5", ControlColor::Alizarin, select1 ); 
  ESPUI.addControl( ControlType::Option, "Cell 6", "6", ControlColor::Alizarin, select1 );
  ESPUI.addControl( ControlType::Option, "Cell 7", "7", ControlColor::Alizarin, select1 );
  ESPUI.addControl( ControlType::Option, "Cell 8", "8", ControlColor::Alizarin, select1 );
  ESPUI.addControl( ControlType::Option, "Cell 9", "9", ControlColor::Alizarin, select1 );
  ESPUI.addControl( ControlType::Option, "Cell 10", "10", ControlColor::Alizarin, select1 );     
  text_cell = ESPUI.addControl( ControlType::Label, "Parametrs in Memory:", "0",ControlColor::Emerald, tab2 );  // id 39
  button_Load = ESPUI.addControl( ControlType::Button, "TẢI TỪ BỘ NHỚ", "LOAD", ControlColor::Wetasphalt, tab2, &buttonExample );   // id 40     
  button_Save = ESPUI.addControl( ControlType::Button, "LƯU VÀO BỘ NHỚ", "SAVE", ControlColor::Wetasphalt, tab2, &buttonExample );  // id 41
  // here tab MMR 2
  select2 = ESPUI.addControl( ControlType::Select, "Select Ball :", "", ControlColor::Peterriver, tab3, &selectExample );  // id 42
  ESPUI.addControl( ControlType::Option, "Ball 1", "1", ControlColor::Peterriver, select2 );
  ESPUI.addControl( ControlType::Option, "Ball 2", "2", ControlColor::Peterriver, select2 );
  ESPUI.addControl( ControlType::Option, "Ball 3", "3", ControlColor::Peterriver, select2 );
  ESPUI.addControl( ControlType::Option, "Ball 4", "4", ControlColor::Peterriver, select2 );
  ESPUI.addControl( ControlType::Option, "Ball 5", "5", ControlColor::Peterriver, select2 );
  ESPUI.addControl( ControlType::Option, "Ball 6", "6", ControlColor::Peterriver, select2 ); 
  ESPUI.addControl( ControlType::Option, "Ball 7", "7", ControlColor::Peterriver, select2 ); 
  ESPUI.addControl( ControlType::Option, "Ball 8", "8", ControlColor::Peterriver, select2 );
  ESPUI.addControl( ControlType::Option, "Ball 9", "9", ControlColor::Peterriver, select2 );
  ESPUI.addControl( ControlType::Option, "Ball 10", "10", ControlColor::Peterriver, select2 );
  select3 = ESPUI.addControl( ControlType::Select, "Memory Cell :", "", ControlColor::Alizarin, tab3, &selectExample );  // id 53
  ESPUI.addControl( ControlType::Option, "Cell 1", "1", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "Cell 2", "2", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "Cell 3", "3", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "Cell 4", "4", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "Cell 5", "5", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "Cell 6", "6", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "Cell 7", "7", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "Cell 8", "8", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "Cell 9", "9", ControlColor::Alizarin, select3 );
  ESPUI.addControl( ControlType::Option, "Cell 10", "10", ControlColor::Alizarin, select3 );
  text_cell_2 = ESPUI.addControl( ControlType::Label, "Ball parametrs in Memory:", "0",ControlColor::Emerald, tab3 );  // id 64
  button_Load_2 = ESPUI.addControl( ControlType::Button, "TẢI TỪ BỘ NHỚ", "LOAD", ControlColor::Wetasphalt, tab3, &buttonExample );   // id 65     
  button_Save_2 = ESPUI.addControl( ControlType::Button, "LƯU VÀO BỘ NHỚ", "SAVE", ControlColor::Wetasphalt, tab3, &buttonExample );  // id 66        
  // here PROGRAM tab
  PROG = ESPUI.addControl( ControlType::Number, "PROGRAMM", "0", ControlColor::Peterriver, tab4, &numberCall ); // id 67
  text_programm = ESPUI.addControl( ControlType::Label, "Programm Parametrs:", "",ControlColor::Wetasphalt, tab4 );  // id 68
  ESPUI.addControl( ControlType::Switcher, "STOP TRỘN BÓNG", "", ControlColor::Turquoise, tab4, &switchExample ); // id 69
  ESPUI.addControl( ControlType::Switcher, "LEDS OFF", "", ControlColor::Wetasphalt, tab4, &switchExample ); // id 70
  
  ESPUI.jsonUpdateDocumentSize = 8000;
  ESPUI.jsonInitialDocumentSize = 32000;

  ESPUI.begin(". Tennis Robot .");
  
 //--------------------------------------------------------------------------------------------------------------

   // ---------------------------- khởi tạo đầu vào diode IR của máy thu ---------------------------------------
   
  // pinMode(Led_Infrared, INPUT);
  // Đếm bóng bằng cách ngắt
  // xử lý - Chức năng Led_S
  // RẤT - khi diode chồng lên nhau sẽ có tín hiệu bằng 0 và chúng ta bắt được
  // attachInterrupt(digitalPinToInterrupt(Led_Infrared), Led_S, FALLING);  //  attachInterrupt(digitalPinToInterrupt(Led_Infrared), Led_S, RISING);
  
  // -----------------------------khởi tạo động cơ và servo-----------------------------------------
  // delay(1000):     // thời gian delay để các động cơ quay
  Motor_1.attach(Motor_1_Pin);
  Motor_2.attach(Motor_2_Pin);
  Motor_3.attach(Motor_3_Pin);
  Servo_L_R.attach(Servo_L_R_Pin);
  Servo_U_D.attach(Servo_U_D_Pin);
  Servo_L_R.write(Servo_L_R_middle);  // đặt servo trái-phải ở vị trí chính giữa
  Servo_U_D.write(Servo_U_D_middle);  // đặt servo lên xuống ở vị trí chính giữa
  Motor_1.writeMicroseconds(Motor_1_stop); //đặt tốc độ trên cùng bên phải ESC = 0
  Motor_2.writeMicroseconds(Motor_2_stop); // đặt tốc độ trên cùng bên trái ESC = 0
  Motor_3.writeMicroseconds(Motor_3_stop); // đặt tốc độ ESC dưới = 0

//-------------------------------- CHỌN CẤU HÌNH Ở ĐÂY ---------------------------------------------------------

 //--------------------Tùy chọn 1 --- động cơ bước-------------------------
 stepper.setMaxSpeed(6800); // đặt giá trị vận tốc stepper tối đa. tốc độ tính bằng bước/giây, tối đa 1000 bước/1s 
 //stepper.setAcceleration(1000);
  //stepper.setSpeed(0); // đặt giá trị mặc định của stepper là 1000 bước / 1s 
 //----------------------------------------------------------------------

 //--------------------TÙY CHỌN 2 - SỬ DỤNG ĐỘNG CƠ GIẢM TỐC -------------------------------------
 // ledcSetup(14, 1024, 8);    // cấu hình xung điều khiển cho động cơ thu (kênh, tần số, độ phân giải)
 // ledcAttachPin(Motor_BallS_Pin, 14); // thiết lập pin, tạo kênhPWM
  
 //----------------------------------------------------------------------------------------------------

  ledcSetup(15, 1024, 8);   // cấu hình xung điều khiển xung cho động cơ chuyển mạch của bi trộn (kênh, tần số, độ phân giải)
  ledcAttachPin(Motor_Mixing_Balls_Pin, 15);  // đặt chốt, kênh tạo ra xung điện xung cho động cơ thu để trộn bóng
  
  delay (3000); // tạm dừng để điều chỉnh ESC

  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);  // Thứ tự Gbps là điển hình
  
  EEPROM.begin(EEPROM_SIZE);
  //Serial.begin(115200);
}


//-------------------------------------------------------------------------------------------------------
// Chức năng xử lý truyền động diode IR

// void Led_S ()
//  {
//   LedInfrared_Flag = true; // khi tín hiệu của diode IR rơi xuống, chúng ta cắm cờ bay của quả 
//  }
//-------------------------------------------------------------------------------------------------------


unsigned long previousDNSMillis = 0;
const unsigned long dnsInterval = 50; // Kiểm tra yêu cầu DNS mỗi 50ms

void loop() {
  // Chỉ xử lý DNS mỗi dnsInterval mili giây
  if (millis() - previousDNSMillis >= dnsInterval) {
    dnsServer.processNextRequest();
    previousDNSMillis = millis();
  }

  // Kiểm tra Stepper
  if (Stepper_Start_Get_Ball && (millis() - stepper_time >= stepper_delay_time)) {
    while (stepper.currentPosition() <= (200 * Stepper_MS)) {
      stepper.setSpeed(set_speed_Balls);
      stepper.runSpeed();
    }
    Serial.println(stepper.speed());
    Stepper_Get_Ball_Flag_Done = true;
    Serial.println("Quay duoc 1 vong");
    stepper.stop();
    stepper_time = millis();
    stepper.setCurrentPosition(0);
  }

  // Tổng số bóng
  ball_S = ball_N1 + ball_N2 + ball_N3 + ball_N4 + ball_N5 + ball_N6 + ball_N7 + ball_N8 + ball_N9 + ball_N10;

  if (Stepper_Get_Ball_Flag_Done) {
    sBall++;
    nBall++;
    random_Flag = true;
    random_Flag_Balls = true;
    Flag_balls_change = true;
    Stepper_Get_Ball_Flag_Done = false;
  }

  // Kiểm tra trạng thái start
  if (!start) {
    Start_Flag = false;
  }

  if (start && !Start_Flag) {
    Start_Flag = true;
    sBall = 0;
    nBall = 0;
    Pausa_Flag = false;
  }

  if (sBall == ball_S) {
    sBall = 0;
  }

  // Xử lý random ball
  if (random_Balls && random_Flag_Balls) {
    sBall = random(0, ball_S);
    random_Flag_Balls = false;
  }

  // Sử dụng một cấu trúc switch hoặc array để giảm code lặp lại
  struct BallConfig {
    int motor_speed1, motor_speed2, motor_speed3, ugol_UD, ugol_LR, random_Ugol, pausa_PRM;
  } ballConfigs[10] = {
    {Motor_1_speed_N1, Motor_2_speed_N1, Motor_3_speed_N1, ugol_U_D_N1, ugol_L_R_N1, random_N1, pausa_N1},
    {Motor_1_speed_N2, Motor_2_speed_N2, Motor_3_speed_N2, ugol_U_D_N2, ugol_L_R_N2, random_N2, pausa_N2},
    {Motor_1_speed_N3, Motor_2_speed_N3, Motor_3_speed_N3, ugol_U_D_N3, ugol_L_R_N3, random_N3, pausa_N3},
    {Motor_1_speed_N4, Motor_2_speed_N4, Motor_3_speed_N4, ugol_U_D_N4, ugol_L_R_N4, random_N4, pausa_N4},
    {Motor_1_speed_N5, Motor_2_speed_N5, Motor_3_speed_N5, ugol_U_D_N5, ugol_L_R_N5, random_N5, pausa_N5},
    {Motor_1_speed_N6, Motor_2_speed_N6, Motor_3_speed_N6, ugol_U_D_N6, ugol_L_R_N6, random_N6, pausa_N6},
    {Motor_1_speed_N7, Motor_2_speed_N7, Motor_3_speed_N7, ugol_U_D_N7, ugol_L_R_N7, random_N7, pausa_N7},
    {Motor_1_speed_N8, Motor_2_speed_N8, Motor_3_speed_N8, ugol_U_D_N8, ugol_L_R_N8, random_N8, pausa_N8},
    {Motor_1_speed_N9, Motor_2_speed_N9, Motor_3_speed_N9, ugol_U_D_N9, ugol_L_R_N9, random_N9, pausa_N9},
    {Motor_1_speed_N10, Motor_2_speed_N10, Motor_3_speed_N10, ugol_U_D_N10, ugol_L_R_N10, random_N10, pausa_N10},
  };

  for (int i = 0, count = 0; i < 10; ++i) {
    count += ball_N[i]; // ball_N là mảng chứa số lượng bóng ở mỗi mức
    if (sBall < count) {
      Motor_1_speed = ballConfigs[i].motor_speed1;
      Motor_2_speed = ballConfigs[i].motor_speed2;
      Motor_3_speed = ballConfigs[i].motor_speed3;
      ugol_UD = ballConfigs[i].ugol_UD;
      ugol_LR = ballConfigs[i].ugol_LR;
      random_Ugol = ballConfigs[i].random_Ugol;
      pausa_PRM = ballConfigs[i].pausa_PRM;
      break;
    }
  }

  // Xử lý ngẫu nhiên và cập nhật góc
  if (random_Flag) {
    UGL_random_R = random(0, UGL_random);
    random_Flag = false;
  }
  UGL_LR = constrain(UGL_LR + ((UGL_LR < 50) ? UGL_random_R : -UGL_random_R), 0, 100);

  Servo_L_R.write(map(UGL_LR, 0, 100, Servo_L_R_min, Servo_L_R_max));
  Servo_U_D.write(map(UGL_UD, 0, 100, Servo_U_D_min, Servo_U_D_max));

  // Chuyển đổi cho Motor 1, 2, 3
  auto mapSpeed = [](int spd, int min_forv, int max_forv, int max_back, int min_back, int stop) {
    if (spd > 0) return map(spd, 1, 100, min_forv, max_forv);
    else if (spd < 0) return map(spd, -100, 1, max_back, min_back);
    return stop;
  };

  set_speed_1 = mapSpeed(SPD_M1, Motor_1_min_forv, Motor_1_max_forv, Motor_1_max_back, Motor_1_min_back, Motor_1_stop);
  set_speed_2 = mapSpeed(SPD_M2, Motor_2_min_forv, Motor_2_max_forv, Motor_2_max_back, Motor_2_min_back, Motor_2_stop);
  set_speed_3 = mapSpeed(SPD_M3, Motor_3_min_forv, Motor_3_max_forv, Motor_3_max_back, Motor_3_min_back, Motor_3_stop);

  if (start) {
    Motor_1.writeMicroseconds(set_speed_1);
    Motor_2.writeMicroseconds(set_speed_2);
    Motor_3.writeMicroseconds(set_speed_3);
  } else {
    Motor_1.writeMicroseconds(Motor_1_stop);
    Motor_2.writeMicroseconds(Motor_2_stop);
    Motor_3.writeMicroseconds(Motor_3_stop);
    Stepper_Start_Get_Ball = false;
  }
}
