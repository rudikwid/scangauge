/**
 * ALEX NEIMAN'S Neiman Mfg. Co (Neimdawg Industries) Real time diagnostics OBD Scan gauge tool.
 * 
 *      
 *      
 *      Z8L NNNN';NNNs qNNN.!NNNa SNNN,~NNNq INNN;'NNNA LNNN>`NNNR TNNNT RNNN`>NNNf ANNN';Nq 
 *    :@@^ ||||`'|||; =|||`'|||! !|||..|||+ ;|||'`|||+ ~|||, |||* ~|||~ *||| ,|||; +|||`a@Q 
 *    _!;                                                                               !!, 
 *    @@Y    !ii+    ,ii;  Liiiiii^ `ii|  `iiL      ;ii<      *ii=     .iiL`   `|ii    +@@' 
 *   'XD,    Q@@@J   X@@< ;@@@%%%%? |@@A  L@@@|    J@@@U     k@@@@.    7@@@R`  +@@B    ?Kw  
 *   IE?    !@@@@@j `@@@` h@@S      g@@^  Q@@@@` `D@@@@|   `%@Q<@@Y    Q@@@@N` d@@c   `6a,  
 *  `@@|    6@@jq@@wL@@m `@@@@@@@J ,@@Q  *@@b@@o~Q@%@@@'  ,Q@Q' Q@Q.  ~@@QL@@Q~@@@`   \@@`  
 *  '!^`   `@@@, K@@@@@; <@@Q,,,,` t@@5 `B@@'X@@@@o~@@Q  _Q@@BKK@@@z  y@@z i@@@@@d    ~!;   
 *  W@X    |@@k   k@@@Q  D@@#jjj{  Q@@_ c@@w 'EEE+ 7@@S ^@@Qiiiij@@Q  @@@.  <@@@@+   ,@@;   
 * `%Q!    |yy,    7yy+  yyyyyyyi .yyn  7yy'       ?yy~'yyy.     yyy''yy7    !yyy    >NN    
 * ^7^                                                                               zi,    
 *`Q@q` :::, .:::. ,::: .:::' ,::: `:::' ':::``:::' ':::` :::, ':::. :::: .:::` ,:::!@@,    
 *,Q@@L`@@@Q }@@@} Q@@@`i@@@w N@@@'^@@@a 6@@@;~@@@K K@@@='@@@N Z@@@T @@@@ J@@@L`Q@@@~aU
 *
 *
 */

// libraries
#include <SPI.h>
#include <mcp2515.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// misc placeholders
#define log_all_codes false
#define PID_RPM 0x0C
#define PID_OutsideTemp 0x46
#define PID_FuelTank 0x2F
#define PID_EngineLoad 0x04
#define PID_CoolantTemp 0x05
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     4 // not sure what this is hahahaha
#define init_delay 3000 // duration that the startup screen is displayed.

// vanity placeholders
#define DEVICE_SERIAL_NUMBER "G0101"
#define DEVICE_NAME "OBD Scan Gauge Device"
#define HARDWARE_VERSION "Hw2.0"
#define DATE_OF_MFG "11/01/2020"
#define FIRMWARE_VERSION "2.0"
#define FIRMWARE_BUILD  "01"

// frame structures
struct can_frame canMsg;
struct can_frame canMsgOutgoing;

// create objects/init board
MCP2515 mcp2515(10);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// timer vars
// max value of an unsigned long is ~2^32...this arduino will run for 50 days and then
// who knows what will happen when the millis() exceeds the max!
unsigned long last_rpm = 0;
unsigned long last_temp = 0;

// display coordinate calculation vars
int16_t xs1, ys1;
uint16_t w, h;

// data vars
int rpm = 9999;
int eng_temp = 999;
int consecutive_ms_button_pressed = 0;
int incoming_byte_serial[11];

 // 'OBD Logo 2 copy', 128x32px ALTERNATE Startup logo (second iteration)
const unsigned char bitmap [] PROGMEM = {
 0x55, 0x45, 0x55, 0x09, 0x22, 0xaa, 0xaa, 0xb6, 0xb7, 0xbd, 0x6a, 0xab, 0x7f, 0xed, 0xef, 0xf7, 
  0x00, 0x10, 0x22, 0xb2, 0xad, 0x55, 0x55, 0x6d, 0x6a, 0xd7, 0xbf, 0xfd, 0xf6, 0xff, 0xff, 0xff, 
  0x2a, 0xa5, 0x44, 0x44, 0x51, 0x2a, 0xaa, 0xca, 0xdd, 0xad, 0x55, 0x57, 0xaf, 0xb7, 0xbb, 0x7f, 
  0x40, 0x08, 0x09, 0x21, 0x00, 0x00, 0x10, 0x35, 0x06, 0xd0, 0xf8, 0x3d, 0x43, 0xfd, 0xff, 0xff, 
  0x15, 0x44, 0x01, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x20, 0x06, 0x00, 0xc0, 0x3f, 0xff, 
  0x40, 0x10, 0x00, 0x80, 0x00, 0x00, 0x00, 0x08, 0x00, 0x80, 0x10, 0x02, 0x00, 0x80, 0x17, 0xef, 
  0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x0e, 0xff, 
  0x50, 0x08, 0xf0, 0x0e, 0x7f, 0xfd, 0xcf, 0x80, 0xf8, 0x1e, 0x03, 0xc0, 0x78, 0x1f, 0x8f, 0xff, 
  0x05, 0x40, 0xf8, 0x1e, 0xff, 0xfd, 0xcf, 0xc1, 0xf8, 0x3f, 0x03, 0xe0, 0x78, 0x3f, 0xcf, 0xff, 
  0x50, 0x11, 0xf8, 0x1e, 0xff, 0xff, 0xdf, 0xc1, 0xf8, 0x3f, 0x07, 0xe0, 0x78, 0x70, 0xc7, 0xbf, 
  0x02, 0xa1, 0xfc, 0x1e, 0xff, 0xfb, 0xdf, 0xc3, 0xf8, 0x7f, 0x07, 0xe0, 0x78, 0x60, 0xc7, 0xef, 
  0x54, 0x01, 0xfc, 0x1e, 0xf0, 0x03, 0xdf, 0xc7, 0xf8, 0x7f, 0x07, 0xf0, 0xf8, 0x63, 0x8f, 0xff, 
  0x02, 0xa1, 0xfe, 0x3f, 0xf0, 0x03, 0xdf, 0xc7, 0xf8, 0xf7, 0x87, 0xf0, 0xf0, 0x7f, 0x8e, 0xff, 
  0x28, 0x01, 0xfe, 0x3d, 0xe0, 0x07, 0xdf, 0xc7, 0xf0, 0xf7, 0x8f, 0xf8, 0xf0, 0x7f, 0x8f, 0xff, 
  0x42, 0xa3, 0xfe, 0x3d, 0xff, 0xf7, 0xff, 0xcf, 0xf1, 0xe7, 0x8f, 0xf8, 0xf0, 0xc0, 0x1f, 0xff, 
  0x14, 0x03, 0xdf, 0x3d, 0xff, 0xf7, 0xbd, 0xcf, 0xf3, 0xe7, 0xcf, 0x7c, 0xf0, 0xc0, 0x3f, 0xff, 
  0x41, 0x43, 0xcf, 0x3d, 0xff, 0xf7, 0xbd, 0xde, 0xf3, 0xc7, 0xcf, 0x3c, 0xf0, 0xff, 0x1d, 0xdf, 
  0x14, 0x03, 0xcf, 0xbf, 0xff, 0xe7, 0xbd, 0xde, 0xf7, 0xc3, 0xcf, 0x3e, 0xe0, 0xff, 0x1f, 0xff, 
  0x21, 0x43, 0xc7, 0xfb, 0xc0, 0x0f, 0xb9, 0xfd, 0xf7, 0xff, 0xdf, 0x3f, 0xe1, 0xc0, 0x1f, 0xff, 
  0x4a, 0x07, 0xc7, 0xfb, 0xc0, 0x0f, 0xf9, 0xfd, 0xef, 0xff, 0xde, 0x1f, 0xe1, 0x80, 0x3f, 0xff, 
  0x01, 0x07, 0x87, 0xfb, 0xc0, 0x0f, 0x79, 0xf9, 0xef, 0xff, 0xde, 0x1f, 0xe0, 0xe6, 0x37, 0x7f, 
  0x54, 0x47, 0x83, 0xfb, 0xc0, 0x0f, 0x79, 0xf9, 0xfe, 0x03, 0xfe, 0x0f, 0xe1, 0xfe, 0x3f, 0xff, 
  0x00, 0x87, 0x81, 0xff, 0xff, 0xef, 0x79, 0xf9, 0xfe, 0x03, 0xfe, 0x0f, 0xc1, 0xbc, 0x3f, 0xff, 
  0x55, 0x07, 0x81, 0xf7, 0xff, 0xff, 0x79, 0xf1, 0xfc, 0x01, 0xfe, 0x07, 0xc3, 0x30, 0x7f, 0xff, 
  0x00, 0x87, 0x01, 0xf3, 0xff, 0xee, 0x71, 0xf1, 0xfc, 0x01, 0xfc, 0x07, 0xc3, 0xf0, 0x77, 0xef, 
  0xaa, 0x03, 0x00, 0xe1, 0xff, 0xce, 0x70, 0xe1, 0xd8, 0x30, 0xdc, 0x03, 0x83, 0xfc, 0x3e, 0xff, 
  0x00, 0x40, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x20, 0x03, 0xfc, 0x7f, 0xff, 
  0x55, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x00, 0xc0, 0x00, 0x00, 0xff, 0xff, 
  0x00, 0xa0, 0x28, 0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x78, 0x00, 0xf0, 0x18, 0x00, 0xff, 0xff, 
  0x55, 0x09, 0x42, 0x95, 0x4a, 0xa9, 0x55, 0x5d, 0x57, 0xae, 0xb7, 0x5d, 0x7e, 0xab, 0xfb, 0xff, 
  0x00, 0x52, 0x2d, 0x2a, 0x55, 0x56, 0xaa, 0xab, 0x6d, 0x7b, 0xed, 0xf7, 0xd7, 0xff, 0xdf, 0x7f, 
  0x55, 0x24, 0x90, 0xa5, 0xaa, 0xa9, 0x55, 0x75, 0xb6, 0xd6, 0xbb, 0x7d, 0xff, 0xff, 0xff, 0xff
};

// 'priusoff', 128x32px
const unsigned char priusoff [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xf0, 0xe1, 0xc0, 0x00, 0x08, 0x00, 0x00, 0x0f, 0xe0, 0xf0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x07, 0xc0, 0xe0, 0x70, 0x00, 0x02, 0x00, 0x00, 0x0f, 0xe1, 0xe0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x1f, 0xc0, 0xe0, 0x1e, 0x00, 0x00, 0x00, 0x07, 0xe1, 0xc1, 0xc0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x37, 0x80, 0xe0, 0x0f, 0x00, 0x00, 0x20, 0x00, 0xc3, 0x83, 0x80, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xc7, 0x80, 0xe0, 0x03, 0xe0, 0x00, 0x0c, 0x01, 0x87, 0x07, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0x00, 0xff, 0xe0, 0x08, 0x78, 0x40, 0x7e, 0x03, 0x0f, 0xef, 0xf8, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0x00, 0x00, 0xff, 0x0e, 0x0e, 0x1f, 0x0f, 0x07, 0xef, 0xef, 0xf8, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1f, 0xc1, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x02, 0x78, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x36, 0xc0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x02, 0xfc, 0x00, 0x00, 0x00, 0xf8, 0x06, 0xa7, 0xfe, 0x40, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x02, 0xec, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x04, 0xee, 0x00, 0x00, 0x01, 0xfe, 0x04, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x05, 0xbe, 0x00, 0x00, 0x03, 0x8f, 0x07, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0x92, 0x00, 0x00, 0x03, 0xef, 0x03, 0x40, 0x07, 0x38, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0xee, 0x00, 0x00, 0x03, 0x6b, 0x03, 0x4f, 0xff, 0xb8, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x8f, 0x00, 0x00, 0x03, 0x03, 0x03, 0x8f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x57, 0xc0, 0x00, 0x03, 0x6f, 0x83, 0x1f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x6d, 0x7f, 0xff, 0x03, 0x47, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x3a, 0xaa, 0xaa, 0xff, 0x93, 0x82, 0xaa, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x14, 0x14, 0x55, 0x7f, 0x97, 0x95, 0x55, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0xef, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x55, 0x55, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};



// icons. Rpm on left, temp on right.
const unsigned char icons [] PROGMEM = {
  0x43, 0xe0, 0x22, 0x00, 0x86, 0xb0, 0x53, 0x00, 0x48, 0x28, 0x02, 0x00, 0x18, 0x4c, 0x03, 0x00, 
  0x10, 0x44, 0x02, 0x00, 0x18, 0x8c, 0x07, 0x00, 0x10, 0x04, 0x97, 0x48, 0x18, 0x0c, 0x67, 0x30, 
  0x08, 0x08, 0x00, 0x00, 0x06, 0x30, 0x92, 0x48, 0x03, 0xe0, 0x6d, 0xb0
};


// function to request OBD data
void requestDataOBD(unsigned long int pid) {
  canMsgOutgoing.can_id  = 0x7DF;   // request
  canMsgOutgoing.can_dlc = 8;       // length of data frame
  canMsgOutgoing.data[0] = 0x02;    // ?
  canMsgOutgoing.data[1] = 0x01;    // ?
  canMsgOutgoing.data[2] = pid;    // OBD PID that we are requesting
  canMsgOutgoing.data[3] = 0x00;   // zeros
  canMsgOutgoing.data[4] = 0x00;
  canMsgOutgoing.data[5] = 0x00;
  canMsgOutgoing.data[6] = 0x00;
  canMsgOutgoing.data[7] = 0x00;
  mcp2515.sendMessage(&canMsgOutgoing);
}

void draw_temp_1() {
  // engine temp
  display.setTextSize(2);
  display.getTextBounds(String(eng_temp), 0, 0, &xs1, &ys1, &w, &h);
  display.setTextWrap(false);
  display.setCursor(130-w,0);
  if (eng_temp != 999) {
    display.print(eng_temp);
  }
}

void draw_ice_1() {
  // rpm number
  display.setTextSize(4);
  display.setTextColor(SSD1306_WHITE);
  display.getTextBounds(String(rpm), 0, 0, &xs1, &ys1, &w, &h);
  display.setCursor(96-w,0);
  display.cp437(true);
  display.print(rpm);
}

void init_screen_1() {
  display.clearDisplay();
  draw_ice_1();
  draw_temp_1();
  display.drawBitmap(98,16,icons,29,11,1);
  display.display();
}

void shut_down() {
  bool sleeping = true;
  consecutive_ms_button_pressed = 0;
  display.clearDisplay();
  display.drawBitmap(0, 0, priusoff, 128, 32, 1);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
  while (true);
  /*
  while (sleeping) {
    if (digitalRead(7)) {
      consecutive_ms_button_pressed++;
      if (consecutive_ms_button_pressed > 300) {
        // Turn display off
        sleeping = false;
        consecutive_ms_button_pressed = 0;
      }
    } else {
      consecutive_ms_button_pressed = 0;
    }
  }*/
}


 void setup() {
  // init screen
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32. Note: if your display does
    for(;;); //                                       not work, try an i2c scanner to find the correct addy.
  }
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap, 128, 32, 1);
  display.display();

  // init can board
  mcp2515.reset();
  mcp2515.setBitrate(CAN_1000KBPS); // Your vehicle may use a different speed!
  mcp2515.setNormalMode();

  // test to see if button pressed
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  pinMode(7, INPUT);

  
  // wait for a second
  for (int x = 0; x < init_delay; x++) {
    delay(1);
    if (digitalRead(7)) {
      consecutive_ms_button_pressed++;
      if (consecutive_ms_button_pressed > 300) {
        // Turn display off
        shut_down();
      }
    }
  }

  
  init_screen_1();
  
  requestDataOBD(PID_CoolantTemp);

}

void loop() {

    if (digitalRead(7)) {
      consecutive_ms_button_pressed++;
      if (consecutive_ms_button_pressed > 3000) {
        // Turn display off
        shut_down();
      }
    } else {
      consecutive_ms_button_pressed = 0;
    }


  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    
    if (canMsg.can_id == 0x7eA) { // response from ecu
  
      if (canMsg.data[2] == PID_CoolantTemp) { // Coolant
        
        eng_temp = (9.0*(canMsg.data[3] - 40)/5.0+32.0); // there are formulas for turning the data bytes
          display.setTextSize(2);                        // into actual "numbers". Look on wikipedia page
          display.getTextBounds(String(eng_temp), 0, 0, &xs1, &ys1, &w, &h);
          display.setTextWrap(false);
          display.fillRect(94,0,36,h,SSD1306_BLACK);
          display.setCursor(130-w,0);
          display.print(eng_temp);
          display.display();
        
      }
      if (canMsg.data[2] == PID_RPM) { // ICE RPM
        rpm = (canMsg.data[3]*256 + canMsg.data[4])/4; // same here. Some codes use more than one byte to store
                                                        // the svalue. The real RPM is a conjugate of two
        display.setTextSize(4);                         // bytes, [3] and [4].
        display.setTextColor(SSD1306_WHITE);
        display.getTextBounds(String(rpm), 0, 0, &xs1, &ys1, &w, &h);
        display.fillRect(0, 0, 94, h, SSD1306_BLACK);
        display.setCursor(96-w,0);
        display.cp437(true);
        display.print(rpm);
        display.display();
      }
      
    }

  }

    // "queue" data update requests
  if ((millis() - last_rpm) > 1430) { // change these values here to change the update frequency
    requestDataOBD(PID_RPM);
    // update counter
    last_rpm = millis();
  }
  if ((millis() - last_temp) > 9930) { // this one too
    requestDataOBD(PID_CoolantTemp);
    // update counter
    last_temp = millis();
  }
  
}
