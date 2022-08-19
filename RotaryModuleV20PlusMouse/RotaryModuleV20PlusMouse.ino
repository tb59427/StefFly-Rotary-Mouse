// A simple Arduino Leonardo based program to use two rotary encoders with push buttons and three additional buttons to operate XCSoar.
// This tutorial was very helpful: http://www.loiph.in/2014/09/arduino-leonardo-atmega32u4-based-usb.html
// Modified by Stefan Langer; www.stefly.aero
// modified by Torsten Beyer to improve debouncing by checking millis between changes in readings
// simple mouse acceleration implemented

// Improvement backlog:
// - double click X to exit xcsoar
// - double click left encoder for flight settings? 


// The rotary encoder library http://www.pjrc.com/teensy/td_libs_Encoder.html
// install this separately via arduino library management
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>


#include <Mouse.h>
#include <Keyboard.h>

// This is an acceleration factor. Use between 1 and 8 to suit how quickly the volume goes up or down.
int accel = 2;

// The digital pin to which the rotary encoder push buttons are connected too.
int Encoder_Button1 = 8;
int Encoder_Button2 = 9;
int Fn_Button = 14;
int QuickMenu = 16;
int ESC = 15;

// debounce delay for key/encoder pushes
int debounce_delay = 300;

// definitions for mouse move (toggled via FN button)
boolean mouse_active = false;
boolean return_to_mouse_mode = false; 
const int Mouse_Move_Distance = 10;


// These are the pins to which the rotary encoder 1 is connected.
// Pins 2,3 are the interrupt pins on a Leonardo/Uno, which give best performance with a rotary encoder.
// Use other pins if you wish, but performance may suffer.
// Avoid using pins that have LED's attached.
// To reverse the direction of the encoder, you only need to change the order of the interrupt pins. E.g. myEnc(3, 2) or myEnc(2, 3)
Encoder myEnc(3, 2);
long oldPosition  = -999;
unsigned long positionExtTime1 = 0;
unsigned long positionExtTimePrev1 = 0;


// These are the pins to which the rotary encoder 2 is connected.
// Pins 0,1 are the interrupt pins on a Leonardo/Uno, which give best performance with a rotary encoder.
// Use other pins if you wish, but performance may suffer.
// Avoid using pins that have LED's attached.
// To reverse the direction of the encoder, you only need to change the order of the interrupt pins. E.g. myEnc2(1, 0) or myEnc2(0, 1)
Encoder myEnc2(1, 0);
long oldPosition2  = -999;
boolean isSTF = false;
unsigned long positionExtTime2 = 0;
unsigned long positionExtTimePrev2 = 0;
unsigned long previousMoveTime1 = 0;
unsigned long currentMoveTime1= 0;
unsigned long previousMoveTime2 = 0;
unsigned long currentMoveTime2 = 0;

// Globs (should really be part of the Encoder class) for debouncing
unsigned long debounceTime = 80;   //min ms between two accepted reads with different reading
unsigned long readingDelta1 = 0;    //holds the time between two consecutive Rotary Encoder1 Readings
unsigned long readingDelta2 = 0;    //holds the time between two consecutive Rotary Encoder1 Readings

unsigned long acceleration = 0;     //current accel based on Encoder acceleration
unsigned long checktime = 0;

#define SUPER_FAST 4
#define FAST 3
#define NORMAL 2
#define SLOW  1

#define FAST_THRESH 200
#define SUPER_FAST_THRESH 150

void setup() {
  pinMode(Encoder_Button1, INPUT_PULLUP);
  pinMode(Encoder_Button2, INPUT_PULLUP);
  pinMode(QuickMenu, INPUT_PULLUP);
  pinMode(ESC, INPUT_PULLUP);
  pinMode(Fn_Button, INPUT_PULLUP);
  delay(1000);
  Mouse.begin();
  //  Serial.begin(57600);
  //  Serial.println("Setup Pushbutton Mouse test:");
}

unsigned long calculateAcceleration(unsigned long previousDelta, unsigned long currentDelta) {

  if (previousDelta > currentDelta) {
    return FAST;
  }
  else if (currentDelta > previousDelta) {
    return SLOW;
  }
  else {
    return NORMAL;
  }
  
}

void loop() {

  // read Encoder 0
  
  long newPosition = myEnc.read() / accel;
  if (newPosition != oldPosition) {
    // Position has changed calculate timing of 2 consecutive reads with changed positions
    checktime = millis();
    // Position has changed calculate timing of 2 consecutive reads with changed positions
    previousMoveTime1 = positionExtTime1 - positionExtTimePrev1;   //record previous time between positive reads
    positionExtTimePrev1 = positionExtTime1;                       //previous reading time becomes Prev time
    positionExtTime1 = checktime;                                   //record this reading time
    currentMoveTime1 = positionExtTime1 - positionExtTimePrev1;    //record time between this read and previous read
    readingDelta1 = checktime - positionExtTimePrev1;

    // if time between changed readings is > debounceTime, we assume a non-bouncing read and do something
    if (readingDelta1 > debounceTime) {
      acceleration = calculateAcceleration (previousMoveTime1,currentMoveTime1);

      if (newPosition > oldPosition) {
        if (mouse_active) {
            Mouse.move(0, -acceleration*Mouse_Move_Distance);
        }
        else {
            Keyboard.write(KEY_UP_ARROW);
        }
      }
      if (oldPosition > newPosition) {
        if (mouse_active) {
            Mouse.move(0, Mouse_Move_Distance);
        }
        else {
            Keyboard.write(KEY_DOWN_ARROW);
        }
      }
    }
    oldPosition = newPosition;
  }

  // read Encoder 1
  
  long newPosition2 = myEnc2.read() / accel;
  if (newPosition2 != oldPosition2) {

    // calculate timing of 2 consecutive reads with changed positions
    checktime = millis();
    previousMoveTime2 = positionExtTime2 - positionExtTimePrev2;
    positionExtTimePrev2 = positionExtTime2;                  //previous reading time becomes Prev time
    positionExtTime2 = checktime;                              //record this reading time
    currentMoveTime2 = positionExtTime2 - positionExtTimePrev2;
    readingDelta2 = checktime - positionExtTimePrev2;

    // if time between changed readings is > debounceTime, we assume a non-bouncing read and do something
    if (readingDelta2 > debounceTime) {

      acceleration = calculateAcceleration (previousMoveTime1,currentMoveTime1);     

      if (newPosition2 > oldPosition2) {
        if (mouse_active) {
            Mouse.move(-acceleration * Mouse_Move_Distance, 0);
        }
        else {
            Keyboard.write(KEY_LEFT_ARROW);
        }
      }
      if (oldPosition2 > newPosition2) {
        if (mouse_active) {
            Mouse.move(Mouse_Move_Distance, 0);
        }
        else {
            Keyboard.write(KEY_RIGHT_ARROW);
        }
      }
    }
    oldPosition2 = newPosition2; 
  }

  if (digitalRead(Encoder_Button1) == LOW) {
    if (mouse_active) {
      Mouse.click();
      delay(debounce_delay);
    } else {
      Keyboard.write(KEY_RETURN);  // send a 'return' to the computer via Keyboard HID
      delay(debounce_delay);  // delay so there aren't a kajillion key presses
    }
  }

  if (digitalRead(Encoder_Button2) == LOW) {
    // Toggle zwischen V (Vario) und S (Sollfahrt)
    if (isSTF) {
      isSTF = false;
      Keyboard.write('V');
    } else {
      isSTF = true;
      Keyboard.write('S');
    }
    //    Keyboard.write(KEY_RETURN);  // send a 'return' to the computer via Keyboard HID
    delay(debounce_delay);  // delay so there aren't a kajillion key presses
  }

  if (digitalRead(QuickMenu) == LOW) {
    Keyboard.write(KEY_F1);  // send a 'F1' to the computer via Keyboard HID to access the Quick Menu in XCSoar
    // switch off mouse mode in case it was on - mouse movements are not helpful in this mode
    if (mouse_active) {
      mouse_active = false;
      return_to_mouse_mode = true;  //remember to go back to mouse mode, if ESC is pressed
    }
    delay(debounce_delay);  // delay so there aren't a kajillion key presses
  }

  if (digitalRead(ESC) == LOW) {
    Keyboard.write(KEY_ESC);  // send a 'ESC' to the computer via Keyboard HID
    if (return_to_mouse_mode) {
      return_to_mouse_mode = false;
      mouse_active = true;
    }
    delay(debounce_delay);  // delay so there aren't a kajillion key presses
  }

  if (digitalRead(Fn_Button) == LOW) {
    mouse_active = !mouse_active;
    //    Keyboard.write(KEY_F5);  // send a 'F5' to the computer via Keyboard HID to access the Waypoints Menu in XCSoar
    delay(debounce_delay);        // delay so there aren't a kajillion key presses
  }
}
