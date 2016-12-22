#include <EEPROM.h> //EEPROM lib for permanent settings
#include <SPI.h> //for display interface
#include <Adafruit_GFX.h> //Graphics lib
#include <Adafruit_PCD8544.h> //display lib
#include "icons.h" //include bitmap file

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 6);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

//factory values
unsigned int tOneshotf = 20000; //time of shutter in oneshot mode
unsigned int tbeforef = 5; //time before shot in oneshot mode
unsigned int delayHDRf = 0; //delay between HDR shots
unsigned int tfocusf = 500; //time of focus button
unsigned int tbetweenf = 30; //time betweense shots
bool wakeUpf = 0; //enable focus button
unsigned int tshotf = 500; //time of shutter in timelapse mode
unsigned int nshotsf = 60; //number of shots in timelapse mode
byte nHDRf = 1; //nHDRshots

//data
unsigned int tOneshot; //time of shutter in oneshot mode
unsigned int tbefore; //time before shot in oneshot mode
unsigned int delayHDR; //delay between HDR shots
unsigned int tfocus; //time of focus button
unsigned int tbetween; //time betweense shots
bool wakeUp; //enable focus button
unsigned int sig = 658696;
unsigned int tshot; //time of shutter in timelapse mode
unsigned int nshots; //number of shots in timelapse mode
byte nHDR; //nHDRshots
//Encoder
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile unsigned int encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent
//unsigned long previousMillis = 0;

//PIN def
int button = 7; //button of rotary encoder
int shotLed = 8; //pin of optocoupler for shutter control
int focusLed = 9; //pin of optocoupler for focus control
int encoderA = 2; //encoder pin A
int encoderB = 3; //encoder pin B

void setup() {
  pinMode(button, INPUT_PULLUP); //init pin 7
  pinMode(shotLed, OUTPUT); //init pin 8
  pinMode(focusLed, OUTPUT); //init pin 9
  pinMode(encoderA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage
  pinMode(encoderB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage
  display.begin(); // init display
  display.clearDisplay();
  display.setContrast(25);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.fillCircle(42, 20, 16, BLACK);
  display.drawLine(29, 16, 29, 24, WHITE);
  display.drawLine(55, 16, 55, 24, WHITE);
  display.drawLine(29, 24, 53, 12, WHITE);
  display.drawLine(55, 16, 32, 28, WHITE);
  display.setCursor(0, 40);
  display.print("interVArduino");
  display.drawBitmap(78, 40, mu_symb, 6, 7, BLACK);
  display.display(); //draw startup logo
  attachInterrupt(0, PinA, RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1, PinB, RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  noInterrupts(); //noInterrupts during eeprom readings
  if(!digitalRead(button)){
    delay(5000);
    if(!digitalRead(button)){
      factory();
    }
  }
  EEPROM.get(0, tshot); //read saved value in eeprom (time of shutter)
  EEPROM.get(2, tfocus);
  EEPROM.get(4, wakeUp); //read saved value in eeprom (enable focus)
  EEPROM.get(6, tOneshot);
  EEPROM.get(8, nshots);
  EEPROM.get(10, delayHDR);
  EEPROM.get(12, tbefore);
  EEPROM.get(14, nHDR);
  EEPROM.get(16, tbetween);
  interrupts(); //now interrupts are enabled
  delay(2000);
  //display.clearDisplay();
}

void loop() {
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);  //DRAW
  display.drawRect(0, 9, 84, 39, BLACK); //Tables
  display.drawLine(42, 9, 42, 48, BLACK);//rows
  display.drawLine(0, 28, 84, 28, BLACK);//and columns of menu
  menu(encoderPos);
  display.display();
  if (encoderPos >= 4 && encoderPos <= 100) encoderPos = 0;
  if (encoderPos <= 65535 && encoderPos >= 65000) encoderPos = 3;
}
void menu(int scelta) {
  switch (scelta) {
    case 0: //Remote control
      display.setCursor(0, 0);
      display.print("Manual");
      display.fillRect(1, 10, 41, 18, BLACK); //<<<<<<<<
      display.drawBitmap(10, 11, manual_symb, 20, 16, WHITE);
      display.drawBitmap(10, 30, camera_symb, 23, 16, BLACK);
      display.drawBitmap(46, 11, timelapse_symb, 33, 16, BLACK);
      display.drawBitmap(55, 30, setting_symb, 18, 16, BLACK);
      if (!digitalRead(button)) {
        bool goOut = 0;
        while (!goOut) {
          display.clearDisplay();
          display.print("Manual");
          if (wakeUp) {
            display.setCursor(70, 0);
            display.setTextColor(WHITE, BLACK);
            display.println(" W");
            display.setTextColor(BLACK, WHITE);
          }
          display.drawLine(0, 9, 84, 9, BLACK);
          display.setCursor(1, 11);
          display.println("When ready, \npress button");
          display.display();
          delay(1000);
          if (wakeUp && !digitalRead(button)) {
            digitalWrite(focusLed, HIGH);
            delay(tfocus);
            digitalWrite(focusLed, LOW);
          }
          unsigned long millisbefore;
          if (!digitalRead(button)) {
            millisbefore = millis();
          }
          while (!digitalRead(button)) {
            display.clearDisplay();
            unsigned long currentMillis = millis();
            display.println("Elapsed time \n(mS):");
            display.print(currentMillis - millisbefore);
            digitalWrite(shotLed, HIGH);
            display.display();
            goOut = 1;
          }
          digitalWrite(shotLed, LOW);
        }
        display.print("\nDone!");
        display.display();
        delay(1500);
        encoderPos = 0;
      }
      break;
    case 1: //timelapse
      display.setCursor(0, 0);
      display.print("Timelapse");
      display.fillRect(43, 10, 41, 18, BLACK); //<<<<<<<<
      display.drawBitmap(10, 11, manual_symb, 20, 16, BLACK);
      display.drawBitmap(10, 30, camera_symb, 23, 16, BLACK);
      display.drawBitmap(46, 11, timelapse_symb, 33, 16, WHITE);
      display.drawBitmap(55, 30, setting_symb, 18, 16, BLACK);
      if (!digitalRead(button)) {
        timelapse();
      }
      break;
    case 2://Oneshot
      display.setCursor(0, 0);
      display.print("One-shot");
      display.fillRect(1, 29, 41, 18, BLACK); //<<<<<<<<
      display.drawBitmap(10, 11, manual_symb, 20, 16, BLACK);
      display.drawBitmap(10, 30, camera_symb, 23, 16, WHITE);
      display.drawBitmap(46, 11, timelapse_symb, 33, 16, BLACK);
      display.drawBitmap(55, 30, setting_symb, 18, 16, BLACK);
      if (!digitalRead(button)) {
        oneshot();
      }
      break;
    case 3: //settings
      display.setCursor(0, 0);
      display.print("Settings");
      display.fillRect(43, 29, 41, 18, BLACK); //<<<<<<<<
      display.drawBitmap(10, 11, manual_symb, 20, 16, BLACK);
      display.drawBitmap(10, 30, camera_symb, 23, 16, BLACK);
      display.drawBitmap(46, 11, timelapse_symb, 33, 16, BLACK);
      display.drawBitmap(55, 30, setting_symb, 18, 16, WHITE);
      if (!digitalRead(button)) {
        settings();
      }
      break;
  }
}

void oneshot() {
  bool goOut = 0;
  unsigned int waiting = tbefore;
  unsigned int tafter = tOneshot;
  encoderPos = 0;
  int selected = 0;
  delay(200);
  while (!goOut) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("One-shot");
    if (wakeUp) {
      display.setCursor(70, 0);
      display.setTextColor(WHITE, BLACK);
      display.println(" W");
      display.setTextColor(BLACK, WHITE);
    }
    display.drawLine(0, 9, 84, 9, BLACK);
    display.setCursor(0, 12);
    display.print("TB S: ");
    display.print(waiting);
    display.setCursor(0, 20);
    if (tafter < 1000) {
      display.print("TS mS: ");
      display.println(tafter);
    }
    else {
      display.print("TS S.: ");
      display.print((tafter / 1000));
      display.print(".");
      display.print(tafter % 1000);
    }
    display.setCursor(36, 40);
    display.print("GO! ");
    display.setCursor(60, 40);
    display.print("Canc");
    if (encoderPos == 4) encoderPos = 0;
    if (encoderPos == 65535) encoderPos = 3;
    switch (encoderPos) {
      case 0:
        display.setCursor(0, 12);
        display.print("TB S: ");
        display.setTextColor(WHITE, BLACK);
        display.println(waiting);
        display.setTextColor(BLACK, WHITE);
        if (!digitalRead(button)) {
          selected = 1;
          int encoderPrew = encoderPos;
          encoderPos = waiting;
          delay(200);
          while (selected == 1) {
            waiting = encoderPos;
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("One-shot");
            if (wakeUp) {
              display.setCursor(70, 0);
              display.setTextColor(WHITE, BLACK);
              display.println(" W");
              display.setTextColor(BLACK, WHITE);
            }
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            display.print("TB S: ");
            display.setTextColor(WHITE, BLACK);
            display.println(waiting);
            display.setTextColor(BLACK, WHITE);
            display.setCursor(0, 20);
            if (tafter < 1000) {
              display.print("TS mS: ");
              display.println(tafter);
            }
            else {
              display.print("TS S.: ");
              display.print((tafter / 1000));
              display.print(".");
              display.print(tafter % 1000); //--------------------------------numeri dopo virgola
            }
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            display.display();
            if (!digitalRead(button)) {
              selected = 0;
              encoderPos = encoderPrew;
              delay(500);
            }
          }
        }
        break;
      case 1:
        display.setCursor(0, 20);
        if (tafter < 1000) {
          display.print("TS mS: ");
          display.setTextColor(WHITE, BLACK);
          display.println(tafter);
          display.setTextColor(BLACK, WHITE);
        }
        else {
          display.print("TS S.: ");
          display.setTextColor(WHITE, BLACK);
          display.print((tafter / 1000));
          display.print(".");
          display.print(tafter % 1000); //--------------------------------numeri dopo virgola
          display.setTextColor(BLACK, WHITE);
        }
        if (!digitalRead(button)) {
          selected = 1;
          int encoderPrew = encoderPos;
          encoderPos = tafter;
          delay(200);
          while (selected == 1) {
            tafter = encoderPos;
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("One-shot");
            if (wakeUp) {
              display.setCursor(70, 0);
              display.setTextColor(WHITE, BLACK);
              display.println(" W");
              display.setTextColor(BLACK, WHITE);
            }
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            display.print("TB S: ");
            display.println(waiting);
            display.setCursor(0, 20);
            if (tafter < 1000) {
              display.print("TS mS: ");
              display.setTextColor(WHITE, BLACK);
              display.println(tafter);
              display.setTextColor(BLACK, WHITE);
            }
            else {
              display.print("TS S.: ");
              display.setTextColor(WHITE, BLACK);
              display.print((tafter / 1000));
              display.print(".");
              display.print(tafter % 1000); //--------------------------------numeri dopo virgola
              display.setTextColor(BLACK, WHITE);
            }
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            display.display();
            if (!digitalRead(button)) {
              selected = 0;
              encoderPos = encoderPrew;
              delay(500);
            }
          }
        }
        break;
      case 2:
        display.setCursor(36, 40);
        display.setTextColor(WHITE, BLACK);
        display.println("Ok");
        display.setTextColor(BLACK, WHITE);
        if (!digitalRead(button)) {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.print("One-shot");
          display.drawLine(0, 9, 84, 9, BLACK);
          if (wakeUp) {
            display.setCursor(70, 0);
            display.setTextColor(WHITE, BLACK);
            display.println(" W");
            display.setTextColor(BLACK, WHITE);
          }
          unsigned long currentw = millis();
          while (millis() - currentw < (waiting * 1000)) {
            display.setCursor(0, 12);
            display.print("Wait: ");
            display.println((float(waiting) - (float(millis() - currentw)) / 1000));
            display.display();
          }
          if (wakeUp) {
            digitalWrite(focusLed, HIGH);
            delay(tfocus);
            digitalWrite(focusLed, LOW);
          }
          unsigned long currenta = millis();
          digitalWrite(shotLed, HIGH);
          while (millis() - currenta < (tafter)) {
            display.setCursor(0, 12);
            display.print("Shoot: ");
            display.println((float(tafter) - float(millis() - currenta)));
            display.display();
          }
          digitalWrite(shotLed, LOW);
          display.clearDisplay();
          display.setCursor(0, 12);
          display.print("Done!");
          display.display();
          delay(2000);
          goOut = 1;
        }
        break;
      case 3:
        display.setCursor(60, 40);
        display.setTextColor(WHITE, BLACK);
        display.println("Canc");
        display.setTextColor(BLACK, WHITE);
        if (!digitalRead(button)) goOut = 1;
        break;
    }
    display.display();
  }
  delay(200);
  encoderPos = 0;
}

void timelapse() {
  bool goOut = 0;
  encoderPos = 0;
  int selected = 0;
  unsigned int nshotsdef = nshots; //number of shots in timelapse mode
  unsigned int tbetweendef = tbetween; //time between shots
  unsigned int tshotdef = tshot; //time of shutter in timelapse mode
  delay(200);
  while (!goOut) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Timelapse");
    if (wakeUp) {
      display.setCursor(70, 0);
      display.setTextColor(WHITE, BLACK);
      display.println(" W");
      display.setTextColor(BLACK, WHITE);
    }
    display.drawLine(0, 9, 84, 9, BLACK);
    display.setCursor(0, 12);
    display.print("N S: ");
    display.println(nshotsdef);
    display.setCursor(0, 20);
    display.print("T B: ");
    display.println(tbetweendef);
    display.setCursor(0, 28);
    if (tshotdef < 1000) {
      display.print("TS mS: ");
      display.println(tshotdef);
    }
    else {
      display.print("TS S.: ");
      display.print((tshotdef / 1000));
      display.print(".");
      display.print(tshotdef % 1000);
    }
    display.setCursor(36, 40);
    display.print("GO! ");
    display.setCursor(60, 40);
    display.print("Canc");
    if (encoderPos == 5) encoderPos = 0;
    if (encoderPos == 65535) encoderPos = 4;
    switch (encoderPos) {
      case 0:
        display.setCursor(0, 12);
        display.print("N S: ");
        display.setTextColor(WHITE, BLACK);
        display.println(nshotsdef);
        display.setTextColor(BLACK, WHITE);
        if (!digitalRead(button)) {
          selected = 1;
          int encoderPrew = encoderPos;
          encoderPos = nshotsdef;
          delay(200);
          while (selected == 1) {
            nshotsdef = encoderPos;
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("Timelapse");
            if (wakeUp) {
              display.setCursor(70, 0);
              display.setTextColor(WHITE, BLACK);
              display.println(" W");
              display.setTextColor(BLACK, WHITE);
            }
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            display.print("N S: ");
            display.setTextColor(WHITE, BLACK);
            display.println(nshotsdef);
            display.setTextColor(BLACK, WHITE);
            display.setCursor(0, 20);
            display.print("T B: ");
            display.println(tbetweendef);
            display.setCursor(0, 28);
            if (tshotdef < 1000) {
              display.print("TS mS: ");
              display.println(tshotdef);
            }
            else {
              display.print("TS S.: ");
              display.print((tshotdef / 1000));
              display.print(".");
              display.print(tshotdef % 1000);
            }
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            display.display();
            if (!digitalRead(button)) {
              selected = 0;
              encoderPos = encoderPrew;
              delay(500);
            }
          }
        }
        break;
      case 1:
        display.setCursor(0, 20);
        display.print("T B: ");
        display.setTextColor(WHITE, BLACK);
        display.println(tbetweendef);
        display.setTextColor(BLACK, WHITE);
        if (!digitalRead(button)) {
          selected = 1;
          int encoderPrew = encoderPos;
          encoderPos = tbetweendef;
          delay(200);
          while (selected == 1) {
            tbetweendef = encoderPos;
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("Timelapse");
            if (wakeUp) {
              display.setCursor(70, 0);
              display.setTextColor(WHITE, BLACK);
              display.println(" W");
              display.setTextColor(BLACK, WHITE);
            }
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            display.print("N S: ");
            display.println(nshotsdef);
            display.setCursor(0, 20);
            display.print("T B: ");
            display.setTextColor(WHITE, BLACK);
            display.println(tbetweendef);
            display.setTextColor(BLACK, WHITE);
            display.setCursor(0, 28);
            if (tshotdef < 1000) {
              display.print("TS mS: ");
              display.println(tshotdef);
            }
            else {
              display.print("TS S.: ");
              display.print((tshotdef / 1000));
              display.print(".");
              display.print(tshotdef % 1000);
            }
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            display.display();
            if (!digitalRead(button)) {
              selected = 0;
              encoderPos = encoderPrew;
              delay(500);
            }
          }
        }
        break;
      case 2:
        display.setCursor(0, 28);
        if (tshotdef < 1000) {
          display.print("TS mS: ");
          display.setTextColor(WHITE, BLACK);
          display.println(tshotdef);
          display.setTextColor(BLACK, WHITE);
        }
        else {
          display.print("TS S.: ");
          display.setTextColor(WHITE, BLACK);
          display.print((tshotdef / 1000));
          display.print(".");
          display.print(tshotdef % 1000);
          display.setTextColor(BLACK, WHITE);
        }
        if (!digitalRead(button)) {
          selected = 1;
          int encoderPrew = encoderPos;
          encoderPos = tshotdef;
          delay(200);
          while (selected == 1) {
            tshotdef = encoderPos;
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("Timelapse");
            if (wakeUp) {
              display.setCursor(70, 0);
              display.setTextColor(WHITE, BLACK);
              display.println(" W");
              display.setTextColor(BLACK, WHITE);
            }
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            display.print("N S: ");
            display.println(nshotsdef);
            display.setCursor(0, 20);
            display.print("T B: ");
            display.println(tbetweendef);
            display.setCursor(0, 28);
            if (tshotdef < 1000) {
              display.print("TS mS: ");
              display.setTextColor(WHITE, BLACK);
              display.println(tshotdef);
              display.setTextColor(BLACK, WHITE);
            }
            else {
              display.print("TS S.: ");
              display.setTextColor(WHITE, BLACK);
              display.print((tshotdef / 1000));
              display.print(".");
              display.print(tshotdef % 1000);
              display.setTextColor(BLACK, WHITE);
            }
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            display.display();
            if (!digitalRead(button)) {
              selected = 0;
              encoderPos = encoderPrew;
              delay(500);
            }
          }
        }
        break;
      case 3:
        display.setCursor(36, 40);
        display.setTextColor(WHITE, BLACK);
        display.println("Ok");
        display.setTextColor(BLACK, WHITE);
        if (!digitalRead(button)) {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.print("Timelapse");
          display.drawLine(0, 9, 84, 9, BLACK);
          if (wakeUp) {
            display.setCursor(70, 0);
            display.setTextColor(WHITE, BLACK);
            display.println(" W");
            display.setTextColor(BLACK, WHITE);
          }
          for (int i = nshotsdef; i > 0; i--) {
              display.setCursor(0, 20);
              display.print("Remain:");
              display.println(i*nHDR);
              display.print("T.tot:");
              display.println(i*nHDR*(tbetweendef+(float(tfocus)/1000)+(float(tshotdef)/1000)));
            unsigned long currentw = millis();
            while (millis() - currentw < (tbetweendef * 1000)) {
              display.setCursor(0, 12);
              display.print("Wait: ");
              display.println((float(tbetweendef) - (float(millis() - currentw)) / 1000));
              display.display();
            }
            if (wakeUp) {
              digitalWrite(focusLed, HIGH);
              delay(tfocus);
              digitalWrite(focusLed, LOW);
            }
            for (int j = nHDR; j > 0; j--) {
              unsigned long currenta = millis();
              digitalWrite(shotLed, HIGH);
              while (millis() - currenta < (tshotdef)) {
                display.setCursor(0, 12);
                display.print("Shoot: ");
                display.println((float(tshotdef) - float(millis() - currenta)));
                display.display();
              }
              digitalWrite(shotLed, LOW);
            }
          }
          display.clearDisplay();
          display.setCursor(0, 12);
          display.print("Done!");
          display.display();
          while(digitalRead(button)){
          }
          goOut = 1; 
        }
        break;
      case 4:
        display.setCursor(60, 40);
        display.setTextColor(WHITE, BLACK);
        display.println("Canc");
        display.setTextColor(BLACK, WHITE);
        if (!digitalRead(button)) goOut = 1;
        break;
    }
    display.display();
  }
  delay(200);
  encoderPos = 0;
}

void settings() {
  int tbeforeOld;
  int tshotOld;
  int nshotOld;
  int tbetweenOld;
  int tfocusOld;
  bool wakeUpOld;
  int nHDRold;
  int delayHDRold;
  int tOneshotOld;

  bool goOut = 0;
  encoderPos = 0;
  delay(500);
  while (!goOut) {
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(0, 0);
    display.println("Settings");
    display.drawLine(0, 9, 84, 9, BLACK);
    display.setCursor(0, 12);
    if (encoderPos == 8) encoderPos = 0;
    if (encoderPos == 65535) encoderPos = 7;
    if (encoderPos < 4) {
      display.println("T. shutter");
      display.println("N. shots");
      display.println("T. between");
      display.println("T&N shots HDR");
    } else {
      display.println("WakeUp, focus");
      display.println("T. one-shot");
      display.println("T. before");
      display.println("Exit");
    }
    switch (encoderPos) {
      case 0:
        tshotOld = tshot;
        display.setTextColor(WHITE, BLACK);
        display.setCursor(0, 12);
        display.println("T. shutter");
        if (!digitalRead(button)) {
          delay(500);
          bool goOut = 0;
          encoderPos = 0;
          int selected = 0;
          while (!goOut) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("T. shutter");
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            if (tshot < 1000) {
              display.print("mS: ");
              display.println(tshot);
            }
            else {
              display.print("S.: ");
              display.print((tshot / 1000));
              display.print(".");
              display.print(tshot); //--------------------------------numeri dopo virgola
            }
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            if (encoderPos == 3) encoderPos = 0;
            if (encoderPos == 65535) encoderPos = 2;
            switch (encoderPos) {
              case 0:
                display.setCursor(0, 12);
                if (tshot < 1000) {
                  display.print("mS: ");
                  display.setTextColor(WHITE, BLACK);
                  display.println(tshot);
                  display.setTextColor(BLACK, WHITE);
                }
                else {
                  display.print("S.: ");
                  display.setTextColor(WHITE, BLACK);
                  display.print((tshot / 1000));
                  display.print(".");
                  display.print(tshot); //--------------------------------numeri dopo virgola
                  display.setTextColor(BLACK, WHITE);
                }
                if (!digitalRead(button)) {
                  selected = 1;
                  int encoderPrew = encoderPos;
                  encoderPos = tshot;
                  delay(200);
                  while (selected == 1) {
                    tshot = encoderPos;
                    display.setCursor(24, 12);
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.setTextColor(BLACK, WHITE);
                    display.println("T. shutter");
                    display.drawLine(0, 9, 84, 9, BLACK);
                    display.setCursor(0, 12);
                    if (tshot < 1000) {
                      display.print("mS: ");
                      display.setTextColor(WHITE, BLACK);
                      display.println(tshot);
                      display.setTextColor(BLACK, WHITE);

                    }
                    else {
                      display.print("S.: ");
                      display.setTextColor(WHITE, BLACK);
                      display.print((tshot / 1000));
                      display.print(".");
                      display.print(tshot);
                      display.setTextColor(BLACK, WHITE);
                      //--------------------------------numeri dopo virgola
                    }
                    display.setCursor(36, 40);
                    display.print("Ok");
                    display.setCursor(60, 40);
                    display.print("Canc");
                    display.display();
                    if (!digitalRead(button)) {
                      selected = 0;
                      encoderPos = encoderPrew;
                      delay(500);
                    }
                  }
                }
                // display.setTextColor(BLACK, WHITE);
                break;
              case 1:
                display.setCursor(36, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Ok");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  goOut = 1;
                  noInterrupts();
                  EEPROM.put(0, tshot);
                  interrupts();
                }
                break;
              case 2:
                display.setCursor(60, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Canc");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  goOut = 1;
                  tshot = tshotOld;
                }
                break;
            }
            display.display();
          }
          delay(200);
          encoderPos = 1;
        }
        break;
      case 1:
        nshotOld = nshots;
        display.setTextColor(WHITE, BLACK);
        display.setCursor(1, 20);
        display.println("N. shots");
        if (!digitalRead(button)) {
          delay(500);
          int selected = 0;
          bool goOut = 0;
          while (!goOut) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("N. shots");
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            display.print("N.: ");
            display.println(nshots);
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            if (encoderPos == 3) encoderPos = 0;
            if (encoderPos == 65535) encoderPos = 2;
            switch (encoderPos) {
              case 0:
                display.setCursor(0, 12);
                display.print("N.: ");
                display.setTextColor(WHITE, BLACK);
                display.println(nshots);
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  selected = 1;
                  int encoderPrew = encoderPos;
                  encoderPos = nshots;
                  delay(200);
                  while (selected == 1) {
                    nshots = encoderPos;
                    display.setCursor(24, 12);
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.setTextColor(BLACK, WHITE);
                    display.println("N. shots");
                    display.drawLine(0, 9, 84, 9, BLACK);
                    display.setCursor(0, 12);
                    display.print("N.: ");
                    display.setTextColor(WHITE, BLACK);
                    display.println(nshots);
                    display.setTextColor(BLACK, WHITE);
                    display.setCursor(36, 40);
                    display.print("Ok");
                    display.setCursor(60, 40);
                    display.print("Canc");
                    display.display();
                    if (!digitalRead(button)) {
                      selected = 0;
                      encoderPos = encoderPrew;
                      delay(500);
                    }
                  }
                }
                //display.setTextColor(BLACK, WHITE);
                break;
              case 1:
                display.setCursor(36, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Ok");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  goOut = 1;
                  noInterrupts();
                  EEPROM.put(8, nshots);
                  interrupts();
                }
                break;
              case 2:
                display.setCursor(60, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Canc");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  goOut = 1;
                  nshots = nshotOld;
                }
                break;
            }
            display.display();
          }
          delay(200);
          encoderPos = 2;
        }
        break;
      case 2:
        tbetweenOld = tbetween;
        display.setTextColor(WHITE, BLACK);
        display.setCursor(1, 27);
        display.println("T. between");
        if (!digitalRead(button)) {
          delay(500);
          bool goOut = 0;
          encoderPos = 0;
          int selected = 0;
          while (!goOut) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("T. between");
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            if (tbetween < 1000) {
              display.print("mS: ");
              display.println(tbetween);
            }
            else {
              display.print("S.: ");
              display.print((tbetween / 1000));
              display.print(".");
              display.print(tbetween); //--------------------------------numeri dopo virgola
            }
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            if (encoderPos == 3) encoderPos = 0;
            if (encoderPos == 65535) encoderPos = 2;
            switch (encoderPos) {
              case 0:
                display.setCursor(0, 12);
                if (tbetween < 1000) {
                  display.print("mS: ");
                  display.setTextColor(WHITE, BLACK);
                  display.println(tbetween);
                  display.setTextColor(BLACK, WHITE);
                }
                else {
                  display.print("S.: ");
                  display.setTextColor(WHITE, BLACK);
                  display.print((tbetween / 1000));
                  display.print(".");
                  display.print(tbetween); //--------------------------------numeri dopo virgola
                  display.setTextColor(BLACK, WHITE);
                }
                if (!digitalRead(button)) {
                  selected = 1;
                  int encoderPrew = encoderPos;
                  encoderPos = tbetween;
                  delay(200);
                  while (selected == 1) {
                    tbetween = encoderPos;
                    display.setCursor(24, 12);
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.setTextColor(BLACK, WHITE);
                    display.println("T. between");
                    display.drawLine(0, 9, 84, 9, BLACK);
                    display.setCursor(0, 12);
                    if (tbetween < 1000) {
                      display.print("mS: ");
                      display.setTextColor(WHITE, BLACK);
                      display.println(tbetween);
                      display.setTextColor(BLACK, WHITE);

                    }
                    else {
                      display.print("S.: ");
                      display.setTextColor(WHITE, BLACK);
                      display.print((tbetween / 1000));
                      display.print(".");
                      display.print(tbetween);
                      display.setTextColor(BLACK, WHITE);
                      //--------------------------------numeri dopo virgola
                    }
                    display.setCursor(36, 40);
                    display.print("Ok");
                    display.setCursor(60, 40);
                    display.print("Canc");
                    display.display();
                    if (!digitalRead(button)) {
                      selected = 0;
                      encoderPos = encoderPrew;
                      delay(500);
                    }
                  }
                }
                //display.setTextColor(BLACK, WHITE);
                break;
              case 1:
                display.setCursor(36, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Ok");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  goOut = 1;
                  noInterrupts();
                  EEPROM.put(16, tbetween);
                  interrupts();
                }
                break;
              case 2:
                display.setCursor(60, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Canc");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  tbetween = tbetweenOld;
                  goOut = 1;
                }
                break;
            }
            display.display();
          }
          delay(200);
          encoderPos = 3;
        }
        break;
      case 3:
        nHDRold = nHDR;
        delayHDRold = delayHDR;
        display.setTextColor(WHITE, BLACK);
        display.setCursor(1, 36);
        display.println("T&N shots HDR");
        if (!digitalRead(button)) {
          delay(500);
          encoderPos = 0;
          int selected = 0;
          delay(500);
          bool goOut = 0;
          while (!goOut) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("T&N shots HDR");
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            display.print("mS: ");
            display.println(delayHDR);
            display.setCursor(0, 20);
            display.print("N.: ");
            display.println(nHDR);
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            if (encoderPos == 4) encoderPos = 0;
            if (encoderPos == 65535) encoderPos = 3;
            switch (encoderPos) {
              case 0:
                display.setCursor(0, 12);
                display.print("mS: ");
                display.setTextColor(WHITE, BLACK);
                display.println(delayHDR);
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  selected = 1;
                  int encoderPrew = encoderPos;
                  encoderPos = delayHDR;
                  delay(200);
                  while (selected == 1) {
                    delayHDR = encoderPos;
                    display.setCursor(24, 12);
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.setTextColor(BLACK, WHITE);
                    display.println("T&N shots HDR");
                    display.drawLine(0, 9, 84, 9, BLACK);
                    display.setCursor(0, 12);
                    display.print("mS: ");
                    display.setTextColor(WHITE, BLACK);
                    display.println(delayHDR);
                    display.setTextColor(BLACK, WHITE);
                    display.setCursor(0, 20);
                    display.print("N.: ");
                    display.println(nHDR);
                    display.setCursor(36, 40);
                    display.print("Ok");
                    display.setCursor(60, 40);
                    display.print("Canc");
                    display.display();
                    if (!digitalRead(button)) {
                      selected = 0;
                      encoderPos = encoderPrew;
                      delay(500);
                    }
                  }
                }
                //display.setTextColor(BLACK, WHITE);
                break;
              case 1:
                display.setCursor(0, 20);
                display.print("N.: ");
                display.setTextColor(WHITE, BLACK);
                display.println(nHDR);
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  selected = 1;
                  int encoderPrew = encoderPos;
                  encoderPos = nHDR;
                  delay(200);
                  while (selected == 1) {
                    nHDR = encoderPos;
                    display.setCursor(24, 12);
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.setTextColor(BLACK, WHITE);
                    display.println("T&N shots HDR");
                    display.drawLine(0, 9, 84, 9, BLACK);
                    display.setCursor(0, 12);
                    display.print("mS: ");
                    display.println(delayHDR);
                    display.setCursor(0, 20);
                    display.print("N.: ");
                    display.setTextColor(WHITE, BLACK);
                    display.println(nHDR);
                    display.setTextColor(BLACK, WHITE);
                    display.setCursor(36, 40);
                    display.print("Ok");
                    display.setCursor(60, 40);
                    display.print("Canc");
                    display.display();
                    if (!digitalRead(button)) {
                      selected = 0;
                      encoderPos = encoderPrew;
                      delay(500);
                    }
                  }
                }
                // display.setTextColor(BLACK, WHITE);
                break;
              case 2:
                display.setCursor(36, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Ok");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  goOut = 1;
                  noInterrupts();
                  EEPROM.put(14, nHDR);
                  EEPROM.put(10, delayHDR);
                  interrupts();
                }
                break;
              case 3:
                display.setCursor(60, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Canc");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  nHDR = nHDRold;
                  delayHDR = delayHDRold;
                  goOut = 1;
                }
                break;
            }
            display.display();
          }
          delay(200);
          encoderPos = 4;
        }
        break;
      case 4:
        tfocusOld = tfocus;
        wakeUpOld = wakeUp;
        display.setTextColor(WHITE, BLACK);
        display.setCursor(0, 12);
        display.println("WakeUp, focus"); //delay shutter
        if (!digitalRead(button)) {
          delay(500);
          encoderPos = 0;
          int selected = 0;
          delay(500);
          bool goOut = 0;
          while (!goOut) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("WakeUp, focus");
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            if (tfocus < 1000) {
              display.print("mS: ");
              display.println(tfocus);
            }
            else {
              display.print("S.: ");
              display.print((tfocus / 1000));
              display.print(".");
              display.print(tfocus); //--------------------------------numeri dopo virgola
            }
            display.setCursor(0, 20);
            display.print("Value: ");
            if (wakeUp) display.print("yes");
            else display.print("no");
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            if (encoderPos == 4) encoderPos = 0;
            if (encoderPos == 65535) encoderPos = 3;
            switch (encoderPos) {
              case 0:
                display.setCursor(0, 12);
                if (tfocus < 1000) {
                  display.print("mS: ");
                  display.setTextColor(WHITE, BLACK);
                  display.println(tfocus);
                  display.setTextColor(BLACK, WHITE);
                }
                else {
                  display.print("S.: ");
                  display.setTextColor(WHITE, BLACK);
                  display.print((tfocus / 1000));
                  display.print(".");
                  display.print(tfocus); //--------------------------------numeri dopo virgola
                  display.setTextColor(BLACK, WHITE);
                }
                if (!digitalRead(button)) {
                  selected = 1;
                  int encoderPrew = encoderPos;
                  encoderPos = tfocus;
                  delay(200);
                  while (selected == 1) {
                    tfocus = encoderPos;
                    display.setCursor(24, 12);
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.setTextColor(BLACK, WHITE);
                    display.println("WakeUp, focus");
                    display.drawLine(0, 9, 84, 9, BLACK);
                    display.setCursor(0, 12);
                    if (tfocus < 1000) {
                      display.print("mS: ");
                      display.setTextColor(WHITE, BLACK);
                      display.println(tfocus);
                      display.setTextColor(BLACK, WHITE);
                    }
                    else {
                      display.print("S.: ");
                      display.setTextColor(WHITE, BLACK);
                      display.print((tfocus / 1000));
                      display.print(".");
                      display.print(tfocus);
                      display.setTextColor(BLACK, WHITE);
                      //--------------------------------numeri dopo virgola
                    }
                    display.setCursor(0, 20);
                    display.print("Value: ");
                    if (wakeUp) display.print("yes");
                    else display.print("no");
                    display.setCursor(36, 40);
                    display.print("Ok");
                    display.setCursor(60, 40);
                    display.print("Canc");
                    display.display();
                    if (!digitalRead(button)) {
                      selected = 0;
                      encoderPos = encoderPrew;
                      delay(500);
                    }
                  }
                }
                // display.setTextColor(BLACK, WHITE);
                break;
              case 1:
                display.setCursor(0, 20);
                display.print("Value: ");
                display.setTextColor(WHITE, BLACK);
                if (wakeUp) display.print("yes");
                else display.print("no");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  selected = 1;
                  int encoderPrew = encoderPos;
                  encoderPos = wakeUp;
                  delay(200);
                  while (selected == 1) {
                    display.setCursor(24, 12);
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.setTextColor(BLACK, WHITE);
                    display.println("WakeUp, focus");
                    display.drawLine(0, 9, 84, 9, BLACK);
                    display.setCursor(0, 12);
                    if (tfocus < 1000) {
                      display.print("mS: ");
                      display.println(tfocus);
                    }
                    else {
                      display.print("S.: ");
                      display.print((tfocus / 1000));
                      display.print(".");
                      display.print(tfocus); //--------------------------------numeri dopo virgola
                    }
                    display.setCursor(0, 20);
                    display.print("Value: ");
                    display.setTextColor(WHITE, BLACK);
                    if (encoderPos % 2) wakeUp = 1;
                    else wakeUp = 0;
                    if (wakeUp) display.print("yes");
                    else display.print("no");
                    display.setTextColor(BLACK, WHITE);
                    display.setCursor(36, 40);
                    display.print("Ok");
                    display.setCursor(60, 40);
                    display.print("Canc");
                    display.display();
                    if (!digitalRead(button)) {
                      selected = 0;
                      encoderPos = encoderPrew;
                      delay(500);
                    }
                  }
                }
                //display.setTextColor(BLACK, WHITE);
                break;
              case 2:
                display.setCursor(36, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Ok");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  goOut = 1;
                  noInterrupts();
                  EEPROM.put(2, tfocus);
                  EEPROM.put(4, wakeUp);
                  interrupts();
                }
                break;
              case 3:
                display.setCursor(60, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Canc");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  wakeUp = wakeUpOld;
                  tfocus = tfocusOld;
                  goOut = 1;
                }
                break;
            }
            display.display();
          }
          delay(200);
          encoderPos = 5;
        }
        break;
      case 5:
        tOneshotOld = tOneshot;
        display.setTextColor(WHITE, BLACK);
        display.setCursor(1, 20);
        display.println("T. one-shot");
        if (!digitalRead(button)) {
          delay(500);
          bool goOut = 0;
          encoderPos = 0;
          int selected = 0;
          while (!goOut) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("T. one-shot");
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            if (tOneshot < 1000) {
              display.print("mS: ");
              display.println(tOneshot);
            }
            else {
              display.print("S.: ");
              display.print((tOneshot / 1000));
              display.print(".");
              display.print(tOneshot); //--------------------------------numeri dopo virgola
            }
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            if (encoderPos == 3) encoderPos = 0;
            if (encoderPos == 65535) encoderPos = 2;
            switch (encoderPos) {
              case 0:
                display.setCursor(0, 12);
                if (tOneshot < 1000) {
                  display.print("mS: ");
                  display.setTextColor(WHITE, BLACK);
                  display.println(tOneshot);
                  display.setTextColor(BLACK, WHITE);
                }
                else {
                  display.print("S.: ");
                  display.setTextColor(WHITE, BLACK);
                  display.print((tOneshot / 1000));
                  display.print(".");
                  display.print(tOneshot); //--------------------------------numeri dopo virgola
                  display.setTextColor(BLACK, WHITE);
                }
                if (!digitalRead(button)) {
                  selected = 1;
                  int encoderPrew = encoderPos;
                  encoderPos = tOneshot;
                  delay(200);
                  while (selected == 1) {
                    tOneshot = encoderPos;
                    display.setCursor(24, 12);
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.setTextColor(BLACK, WHITE);
                    display.println("T. one-shot");
                    display.drawLine(0, 9, 84, 9, BLACK);
                    display.setCursor(0, 12);
                    if (tOneshot < 1000) {
                      display.print("mS: ");
                      display.setTextColor(WHITE, BLACK);
                      display.println(tOneshot);
                      display.setTextColor(BLACK, WHITE);

                    }
                    else {
                      display.print("S.: ");
                      display.setTextColor(WHITE, BLACK);
                      display.print((tOneshot / 1000));
                      display.print(".");
                      display.print(tOneshot);
                      display.setTextColor(BLACK, WHITE);
                      //--------------------------------numeri dopo virgola
                    }
                    display.setCursor(36, 40);
                    display.print("Ok");
                    display.setCursor(60, 40);
                    display.print("Canc");
                    display.display();
                    if (!digitalRead(button)) {
                      selected = 0;
                      encoderPos = encoderPrew;
                      delay(500);
                    }
                  }
                }
                //display.setTextColor(BLACK, WHITE);
                break;
              case 1:
                display.setCursor(36, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Ok");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  goOut = 1;
                  noInterrupts();
                  EEPROM.put(6, tOneshot);
                  interrupts();
                }
                break;
              case 2:
                display.setCursor(60, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Canc");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  tOneshot = tOneshotOld;
                  goOut = 1;
                }
                break;
            }
            display.display();
          }
          delay(200);
          encoderPos = 6;
        }
        break;
      case 6:
        tbeforeOld = tbefore;
        display.setTextColor(WHITE, BLACK);
        display.setCursor(1, 27);
        display.println("T. before");
        if (!digitalRead(button)) {
          delay(500);
          bool goOut = 0;
          encoderPos = 0;
          int selected = 0;
          while (!goOut) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextColor(BLACK, WHITE);
            display.println("T. before");
            display.drawLine(0, 9, 84, 9, BLACK);
            display.setCursor(0, 12);
            display.print("S.: ");
            display.print(tbefore);
            display.setCursor(36, 40);
            display.print("Ok");
            display.setCursor(60, 40);
            display.print("Canc");
            if (encoderPos == 3) encoderPos = 0;
            if (encoderPos == 65535) encoderPos = 2;
            switch (encoderPos) {
              case 0:
                display.setCursor(0, 12);
                display.print("S.: ");
                display.setTextColor(WHITE, BLACK);
                display.print(tbefore);
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  selected = 1;
                  int encoderPrew = encoderPos;
                  encoderPos = tbefore;
                  delay(200);
                  while (selected == 1) {
                    tbefore = encoderPos;
                    display.setCursor(24, 12);
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.setTextColor(BLACK, WHITE);
                    display.println("T. before");
                    display.drawLine(0, 9, 84, 9, BLACK);
                    display.setCursor(0, 12);
                    display.print("S.: ");
                    display.setTextColor(WHITE, BLACK);
                    display.print(tbefore);
                    display.setTextColor(BLACK, WHITE);
                    display.setCursor(36, 40);
                    display.print("Ok");
                    display.setCursor(60, 40);
                    display.print("Canc");
                    display.display();
                    if (!digitalRead(button)) {
                      selected = 0;
                      encoderPos = encoderPrew;
                      delay(500);
                    }
                  }
                }
                //display.setTextColor(BLACK, WHITE);
                break;
              case 1:
                display.setCursor(36, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Ok");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  goOut = 1;
                  noInterrupts();
                  EEPROM.put(12, tbefore);
                  interrupts();
                }
                break;
              case 2:
                display.setCursor(60, 40);
                display.setTextColor(WHITE, BLACK);
                display.println("Canc");
                display.setTextColor(BLACK, WHITE);
                if (!digitalRead(button)) {
                  tbefore = tbeforeOld;
                  goOut = 1;
                }
                break;
            }
            display.display();
          }
          delay(200);
          encoderPos = 7;
        }
        break;
      case 7:
        display.setTextColor(WHITE, BLACK);
        display.setCursor(1, 36);
        display.println("Exit");
        if (!digitalRead(button)) {
          goOut = 1;
        }
        break;
    }
    display.display();
  }
  encoderPos = 0;
  delay(200);
}

void factory(){
  EEPROM.put(0, tshotf);
  EEPROM.put(2, tfocusf);
  EEPROM.put(4, wakeUpf);
  EEPROM.put(6, tOneshotf);
  EEPROM.put(8, nshotsf);
  EEPROM.put(10, delayHDRf);
  EEPROM.put(12, tbeforef);
  EEPROM.put(14, nHDRf);
  EEPROM.put(16, tbetweenf);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("DONE! Factory restored. Please, restart device.");
  display.display();
  while(1){
  }
}
void PinA() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}
