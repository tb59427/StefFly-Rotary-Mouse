// A simple Arduino Leonardo based program to use two rotary encoders with push buttons and three additional buttons to operate XCSoar.
// This tutorial was very helpful: http://www.loiph.in/2014/09/arduino-leonardo-atmega32u4-based-usb.html
// Modified by Stefan Langer; www.stefly.aero
// modified by Torsten Beyer to improve debouncing by checking millis between changes in readings
// linear mouse acceleration implemented


// The rotary encoder library http://www.pjrc.com/teensy/td_libs_Encoder.html
// install this separately via arduino library management
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>


#include <Mouse.h>
#include <Keyboard.h>

// GOBAL DEFINITIONS - change these for different behaviour
//
// min ms between two accepted reads with different reading - CHANGE this for different timing
#define DEBOUNCETIME 25

// delay used to debounce button presses - this is still based on old code with delay. Will have to be changed to timing based
#define DEBOUNCEDELAY 300

//min mouse move distance
#define MOUSEMOVEDISTANCE 10

//max mouse move acceleration - mouse will be moved times this factor in max accel
#define MAXMOUSEACCEL 4

//max time between rotary reads considered in accel calculations in ms
#define LONGCUTOFF 200

//min time between rotary reads considered in accel calculation in ms
#define SHORTCUTOFF DEBOUNCETIME

// DO NOT TOUCH ANYTHING BEYOND THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING :-)

// This is an acceleration factor. Use between 1 and 8 to suit how quickly the volume goes up or down.
int accel = 2;

// The digital pin to which the rotary encoder push buttons are connected too.
int Encoder_Button1 = 8;
int Encoder_Button2 = 9;
int Fn_Button = 14;
int QuickMenu = 16;
int ESC = 15;

// debounce delay for key pushes
int debounce_delay = DEBOUNCEDELAY;

// definitions for mouse move (toggled via FN button)
boolean mouse_active = false;
boolean return_to_mouse_mode = false; 
unsigned long mouseticks = 0;         //mouse move distance as calculate after each read (in mouse mode)

// Variables for mouse acceleration calculation
//
// Stuff for acceleration of mouse pointer 
// at 200ms or slower, there should be no acceleration.
// CHANGE if you want different timing
constexpr float longCutoff = LONGCUTOFF;
constexpr float minMouseMove = MOUSEMOVEDISTANCE;

// at DEBOUNCETIME ms, we want to have maximum mouse move distance
// CHANGE if you want different timing
constexpr float shortCutoff = DEBOUNCETIME;

// accelerate to max 4*Mouse_Move_Distance - CHANGE if you want different behaviour
constexpr float maxMouseMove = MAXMOUSEACCEL*MOUSEMOVEDISTANCE;

// To derive the calc. constants, compute as follows:
// Resolve a linear formular f(x) = a * x + b;
// f(x) delivers the mouse move distance for a given timing of x (time between 2 successive reads)
// f(x) with x > longCutOff is fixed at minMouseMove / f(x) with x < shortCutoff is fixed at maxMouseMove
// in other words: no acceleration beyon shortCutoff and no deceleration beyond longCutoff
// where  f(longCutoff)=minMouseMove and f(shortCutoff)=maxMouseMove
// ONLY CHANGE if you want different behaviour (i.e. non-linear acceleration between longCutoff and shortCutoff

constexpr float a = (maxMouseMove - minMouseMove)/(shortCutoff - longCutoff);
constexpr float b = shortCutoff - a*maxMouseMove;


// These are the pins to which the rotary encoder 0 is connected.
// Pins 2,3 are the interrupt pins on a Leonardo/Uno, which give best performance with a rotary encoder.
// Use other pins if you wish, but performance may suffer.
// Avoid using pins that have LED's attached.
// To reverse the direction of the encoder, you only need to change the order of the interrupt pins. E.g. myEnc(3, 2) or myEnc(2, 3)

Encoder myEnc0(3, 2);
long oldPosition  = -999;
unsigned long positionTime1 = 0;
unsigned long positionTimePrev1 = 0; 

// These are the pins to which the rotary encoder 1 is connected.
// Pins 0,1 are the interrupt pins on a Leonardo/Uno, which give best performance with a rotary encoder.
// Use other pins if you wish, but performance may suffer.
// Avoid using pins that have LED's attached.
// To reverse the direction of the encoder, you only need to change the order of the interrupt pins. E.g. myEnc2(1, 0) or myEnc2(0, 1)

Encoder myEnc1(1, 0);
long oldPosition2  = -999;
boolean isSTF = false;
unsigned long positionTime2 = 0;
unsigned long positionTimePrev2 = 0;

// Globs (should really be part of the Encoder class) for debouncing
unsigned long readingDelta1 = 0;    //holds the time between two consecutive Rotary Encoder1 Readings
unsigned long readingDelta2 = 0;    //holds the time between two consecutive Rotary Encoder1 Readings

unsigned long checktime = 0;

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

unsigned long calculateAcceleration(unsigned long currentDelta) {

    unsigned long ms = currentDelta;
    float ticks;
    
    if (ms < longCutoff) {
      // do some acceleration using factors a and b

      // limit to maximum acceleration
      if (currentDelta < shortCutoff) {
        ms = shortCutoff;
      }

      // calculate ticks derived from linear function
      ticks = a*ms + b;

      // return ticks - but cast to unsigned long as mouse can not move fractions
      return (unsigned long) ticks;

    }
    else {
      return MOUSEMOVEDISTANCE;
    }

}

void loop() {

  // read Encoder 0
  
  long newPosition = myEnc0.read() / accel;
  if (newPosition != oldPosition) {
    // Position has changed calculate timing of 2 consecutive reads with changed positions
    checktime = millis();
    // Position has changed calculate timing of 2 consecutive reads with changed positions
    positionTimePrev1 = positionTime1;                       //previous reading time becomes Prev time
    positionTime1 = checktime;                                  //record this reading time
    readingDelta1 = checktime - positionTimePrev1;

    // if time between changed readings is > DEBOUNCETIME, we assume a non-bouncing read and do something
    if (readingDelta1 > DEBOUNCETIME) {
      mouseticks = calculateAcceleration (readingDelta1);

      if (newPosition > oldPosition) {
        if (mouse_active) {
            Mouse.move(0, -mouseticks);
        }
        else {
            Keyboard.write(KEY_UP_ARROW);
        }
      }
      if (oldPosition > newPosition) {
        if (mouse_active) {
            Mouse.move(0, mouseticks);
        }
        else {
            Keyboard.write(KEY_DOWN_ARROW);
        }
      }
    }
    oldPosition = newPosition;
  }

  // read Encoder 1
  
  long newPosition2 = myEnc1.read() / accel;
  if (newPosition2 != oldPosition2) {

    // calculate timing of 2 consecutive reads with changed positions
    checktime = millis();
    positionTimePrev2 = positionTime2;                   //previous reading time becomes Prev time
    positionTime2 = checktime;                              //record this reading time
    readingDelta2 = checktime - positionTimePrev2;

    // if time between changed readings is > DEBOUNCETIME, we assume a non-bouncing read and do something
    if (readingDelta2 > DEBOUNCETIME) {

      mouseticks = calculateAcceleration (readingDelta2);     

      if (newPosition2 > oldPosition2) {
        if (mouse_active) {
            Mouse.move(-mouseticks, 0);
        }
        else {
            Keyboard.write(KEY_LEFT_ARROW);
        }
      }
      if (oldPosition2 > newPosition2) {
        if (mouse_active) {
            Mouse.move(mouseticks, 0);
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
      //delay(debounce_delay);
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
