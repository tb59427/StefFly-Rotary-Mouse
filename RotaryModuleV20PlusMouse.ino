// A simple Arduino Leonardo based program to use two rotary encoders with push buttons and three additional buttons to operate XCSoar. 
// This tutorial was very helpful: http://www.loiph.in/2014/09/arduino-leonardo-atmega32u4-based-usb.html
// Modified by Stefan Langer; www.stefly.aero

#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
// The rotary encoder library http://www.pjrc.com/teensy/td_libs_Encoder.html

#include <Mouse.h>
#include <Keyboard.h>

int accel = 2;
// This is an acceleration factor. Use between 1 and 8 to suit how quickly the volume goes up or down.

int Encoder_Button1 = 8;
int Encoder_Button2 = 9;
// The digital pin to which the rotary encoder push buttons are connected too.

int Fn_Button = 14;
int QuickMenu = 16;
int ESC = 15;
int debounce_delay = 300;
boolean mouse_active = 0;
const int Mouse_Move_Distance = 10;

Encoder myEnc(3, 2);
// These are the pins to which the rotary encoder 1 is connected.
// Pins 2,3 are the interrupt pins on a Leonardo/Uno, which give best performance with a rotary encoder.
// Use other pins if you wish, but performance may suffer.
// Avoid using pins that have LED's attached.
// To reverse the direction of the encoder, you only need to change the order of the interrupt pins. E.g. myEnc(3, 2) or myEnc(2, 3)
long oldPosition  = -999;
int buttonState;
int lastButtonState = LOW;
long lastDebounceTime = 0;
long debounceDelay = 120;  // Button debounce time in millseconds. Increase if mute button doesnt work properly.

Encoder myEnc2(1, 0);
// These are the pins to which the rotary encoder 1 is connected.
// Pins 0,1 are the interrupt pins on a Leonardo/Uno, which give best performance with a rotary encoder.
// Use other pins if you wish, but performance may suffer.
// Avoid using pins that have LED's attached.
// To reverse the direction of the encoder, you only need to change the order of the interrupt pins. E.g. myEnc2(1, 0) or myEnc2(0, 1)
long oldPosition2  = -999;
int buttonState2;
int lastButtonState2 = LOW;
long lastDebounceTime2 = 0;
long debounceDelay2 = 120;
boolean isSTF = false;

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

void loop() {
  if (mouse_active){
    long newPosition = myEnc.read() / accel;
    if (newPosition != oldPosition) {
      if (newPosition > oldPosition) {
        Mouse.move(0, -Mouse_Move_Distance);
//        Serial.println("Maus 10 nach oben...");
        delay(debounceDelay);
      }
      if (oldPosition > newPosition) {
        Mouse.move(0, Mouse_Move_Distance);
//        Serial.println("Maus 10 nach unten...");
        delay(debounceDelay);
      }
      oldPosition = newPosition;
      myEnc.read(); // trash one read to reduce bouncing
    }
    long newPosition2 = myEnc2.read() / accel;
    if (newPosition2 != oldPosition2) {
      if (newPosition2 > oldPosition2) {
        Mouse.move(-Mouse_Move_Distance, 0);
//        Serial.println("Maus 10 nach links...");
        delay(debounceDelay);
      }
      if (oldPosition2 > newPosition2) {
        Mouse.move(Mouse_Move_Distance, 0);
//        Serial.println("Maus 10 nach rechts...");
        delay(debounceDelay);
      }
      oldPosition2 = newPosition2;
      myEnc2.read(); // trash one read to reduce bouncing
    }
  } else {
    long newPosition = myEnc.read() / accel;
    if (newPosition != oldPosition) {
      if (newPosition > oldPosition) {
        Keyboard.write(KEY_UP_ARROW);
      }
      if (oldPosition > newPosition) {
        Keyboard.write(KEY_DOWN_ARROW);
      }
      delay(debounce_delay);  // delay so there aren't a kajillion key presses
      oldPosition = newPosition;
      myEnc.read(); // trash one read to reduce bouncing
    }
    long newPosition2 = myEnc2.read() / accel;
    if (newPosition2 != oldPosition2) {
      if (newPosition2 > oldPosition2) {
        Keyboard.write(KEY_LEFT_ARROW);  
      }
      if (oldPosition2 > newPosition2) {
        Keyboard.write(KEY_RIGHT_ARROW);
      }
      delay(debounce_delay);  // delay so there aren't a kajillion key presses
      oldPosition2 = newPosition2;
      myEnc2.read(); // trash one read to reduce bouncing
    }
  }

  if (digitalRead(Encoder_Button1) == LOW) {
    if (mouse_active){
      Mouse.click();
      delay(debounce_delay);
    } else {
      Keyboard.write(KEY_RETURN);  // send a 'return' to the computer via Keyboard HID
      delay(debounce_delay);  // delay so there aren't a kajillion key presses
    }
  }

  if (digitalRead(Encoder_Button2) == LOW){
    // Toggle zwischen V (Vario) und S (Sollfahrt)
    if (isSTF) {
      isSTF = false;
      Keyboard.write('V');
//      Serial.println("Vario mode...");
    } else {
      isSTF = true;
      Keyboard.write('S');
//      Serial.println("Sollfahrt mode...");
    }
//    Keyboard.write(KEY_RETURN);  // send a 'return' to the computer via Keyboard HID
    delay(debounce_delay);  // delay so there aren't a kajillion key presses
  }

  if (digitalRead(QuickMenu) == LOW){
    Keyboard.write(KEY_F1);  // send a 'F1' to the computer via Keyboard HID to access the Quick Menu in XCSoar
    delay(debounce_delay);  // delay so there aren't a kajillion key presses
  }
  
  if (digitalRead(ESC) == LOW){
    Keyboard.write(KEY_ESC);  // send a 'ESC' to the computer via Keyboard HID 
    delay(debounce_delay);  // delay so there aren't a kajillion key presses
  }
  
  if (digitalRead(Fn_Button) == LOW){
    mouse_active = !mouse_active;
//    Keyboard.write(KEY_F5);  // send a 'F5' to the computer via Keyboard HID to access the Waypoints Menu in XCSoar
    delay(debounce_delay);  // delay so there aren't a kajillion key presses
//    if (mouse_active){
//      Serial.println("Maus jetzt aktiv...");
//    } else {
//      Serial.println("Keyboard jetzt aktiv...");
//    }
  }
}
