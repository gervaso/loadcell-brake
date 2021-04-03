#include <Arduino.h>
#include <EEPROM.h>
#include "HX711.h"
#include "Joystick.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
const int BIND_MODE_PIN = 8; // The arduino pin to check to enable binding mode
const int RESET_ZERO = 9; // The arduino pin to ZERO the load cell
const bool INVERTED_LOAD = 0;

HX711 loadcell;

int loadcell_value = 0;
bool bind_mode = 0;
int address = 0;

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
                   32, 0,                  // Button Count, Hat Switch Count
                   false, false, false,     // X and Y, but no Z Axis
                   false, false, false,   // No Rx, Ry, or Rz
                   false, false,          // No rudder or throttle
                   false, true, false);  // No accelerator, brake, or steering

void binding_mode() {
  /*
    Here it will simulate pushing the loadCell assigned pedal to bind the axis
    in game without having to actually push the pedal.
    That'll save you having to mod your existing pedals just to assign the pedal
    axis correctly in game

    It will push the pedal twice... you can change the cycles from 2 to more if
    you like.
    
  */

  for (int cycles = 0; cycles < 2; cycles++) { // Push the virtual pedal twice
    for (int i = 0; i <= 1023; i++) { // Pedal down
      Joystick.setBrake(i);  // Make this your brake axis if you need to
      delay(2); // small delay, Should take 2 seconds to pedal down
      //Joystick.sendState(); // Change this for your own joystick library update if you need to
    }
    for (int i = 1023; i >= 0; i--) { // Pedal up
      Joystick.setBrake(i); // Make this your brake axis if you need to
      delay(2); // small delay, Should take 2 seconds to pedal up
      //Joystick.sendState(); // Change this for your own joystick library update if you need to
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Begin ");
  pinMode(BIND_MODE_PIN, INPUT_PULLUP); // Binding button...
  pinMode(RESET_ZERO, INPUT_PULLUP); // Reset Zero button...
  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  int sensitivity = EEPROM.read(address);

  // **** First boot check the saved EEPROM setting for sensitivity settings. If it's not 64 or 128, set it and save it for next boot. ****

  if (!(sensitivity == 64) || (sensitivity == 128)) {
    sensitivity = 64;
    loadcell.set_gain(sensitivity);  // Initialise to Low sensitivity and heck, save it for next boot so we don't have to do this again.
    EEPROM.write(address, sensitivity); // save the sensitivity to the EEPROM for next boot
    delay(50);// Let the EEPROM stabilise before reading it.

  }// end first boot settings

  // Setup the Load Cell settings
  Serial.println("EEPROM Sensitivity is ");
  Serial.print(sensitivity);
  loadcell.set_gain(sensitivity);  // Get the EEPROM from
  loadcell.set_scale(-1000);  // Puts the numbers neatly into the 0-1023 range.
  loadcell.tare();       // Reset values to zero

  Joystick.begin();
  Joystick.setBrake(0);
}

void loop() {

  if (!digitalRead(BIND_MODE_PIN)) { // Checks if the Binding Mode Button has been pressed. Active Low.
    Serial.println("Moving the pedal for you now...");
    binding_mode(); // binds the axis for a game for you...
    Serial.println("Done.");
  }

  if (!digitalRead(RESET_ZERO)) { // Checks if the Binding Mode Button has been pressed. Active Low.
    Serial.println("Reset Zero Load Cell Value");
    delay(1000);
    loadcell.tare();       // Reset scale to zero
    Serial.println("Done.");
  }

  if (loadcell.is_ready()) {
    //long reading = loadcell.read();
    //Serial.print("HX711 reading: ");
    //Serial.println(reading);
    // This value you can put into your joystick axis before updating the PC
    //loadcell_value = abs(loadcell.get_units(1));
    if (INVERTED_LOAD) {
      loadcell_value = (loadcell.get_units(1) * -1); 
    } else {
      loadcell_value = loadcell.get_units(1);
    }
    Joystick.setBrake(constrain(loadcell_value, 0, 1023));   // Put load cell value into rudder axis - Change this for your Joystick preferred axis. Don't forget to constrain it into a 10bit container ;)
  } //else {
    //Serial.println("HX711 not found.");
    //delay(1000);
  //}

  //delay(200);
  
}


