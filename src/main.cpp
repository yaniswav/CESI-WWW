#include <ChainableLED.h>
#include <Arduino.h>

#define NUM_LEDS 1
ChainableLED leds(7, 8, NUM_LEDS);

#define GREEN_BUTTON 3
#define RED_BUTTON 2

#define MODE_NORMAL 0
#define MODE_ECO 1
#define MODE_MAINTENANCE 2
#define MODE_CONFIG 3
#define MAX_VALUE 3

byte previousMode = MODE_NORMAL;
byte mode = MODE_NORMAL;

//void checkButton();

unsigned long buttonPressedMs = millis();
bool buttonPressed = false;
bool checkStartPressedButton = true;

void red_button_event(){
  Serial.println("Pression rouge");
  buttonPressedMs = millis();
  buttonPressed = true;
 
}

void green_button_event(){
  Serial.println("Pression vert");
  buttonPressedMs = millis();
  buttonPressed = true;
}

void changeMode(int mode_name) {
  
  String name = F("");

  switch (mode_name) {
    case MODE_NORMAL:
      name = F("normal");
      break;
    case MODE_ECO:
      name = F("eco");
      break;
    case MODE_MAINTENANCE:
      name = F("maintenance");
      break;
    case MODE_CONFIG:
      name = F("configuration");
      break;
  }
  Serial.print(F("The new mode is: "));
  Serial.println(name);
}

void pressedButtonGreen() {
  leds.setColorHSL(0.0, 0.0, 0.0, 0.0);
  // changeMode(MODE_ECO);
}

void pressedButtonRed() {

  leds.setColorHSL(0.8, 1.0, 1.0, 0.5);
  // changeMode(MODE_NORMAL);

}

void checkButton(){
      //Serial.println(buttonPressed);
  Serial.println(millis() - buttonPressedMs);
    
  if (checkStartPressedButton && digitalRead(RED_BUTTON) == 0) {
    // go into config mode
    leds.setColorHSL(0.2, 0.2, 0.7, 0.5);
    changeMode(MODE_CONFIG);
  } checkStartPressedButton = false;

    if ((millis() - buttonPressedMs) > (2 * 1000) && buttonPressed) {
      Serial.println("yo");
      if (digitalRead(RED_BUTTON) == 0) {
        Serial.println("avant le write high");
        pressedButtonRed();
      } else if (digitalRead(GREEN_BUTTON) == 0) {
        Serial.println("avant le write LOW");
        pressedButtonGreen();
      }
      buttonPressed = false;
    }
}    
        
void setup()
{
  leds.init();
  Serial.begin(9600);
  //pinMode(LEDPIN, OUTPUT); // Initialisation LED
  pinMode(GREEN_BUTTON, INPUT_PULLUP); // Initialisation bouton
  pinMode(RED_BUTTON, INPUT_PULLUP); // Initialisation bouton
  
  attachInterrupt(digitalPinToInterrupt(RED_BUTTON), red_button_event, FALLING);
  attachInterrupt(digitalPinToInterrupt(GREEN_BUTTON), green_button_event, FALLING);
}

void loop()
{
  checkButton();
}

