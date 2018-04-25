#include <Arduino.h>
#include <SigFox.h>
#include <ButtonPress.h>
#include <TemperatureSensor.h>
#include <VibrationSensor.h>
#include <SigfoxMessage.h>


const int TIME_SECOND = 1000;
const int TIME_MINUTE = TIME_SECOND * 60;
const int TIME_HOUR = TIME_MINUTE * 60;
const int HEARTBEAT_TIME_MINUTES = 10;


const int BUTTON_PIN = 7;
const int BUTTON_INDEX = 0;
ButtonPress buttonPress;

const int TEMP_PIN = 6;
const int TEMP_INDEX = 1;
OneWire oneWire(TEMP_PIN); 
DallasTemperature sensors(&oneWire);
TemperatureSensor temperatureSensor; 

const int VIBRATION_PIN = A0;
const int VIBRATION_INDEX = 2;
VibrationSensor vibrationSensor;

SigfoxMessage sigfox;
const int MAX_MESSAGE_LENGTH = 12;

bool checkButtonPressed() {
  bool pressed = false;
  int buttonPressed = buttonPress.isPressed();
  
  if (buttonPressed == 1) 
  {
    pressed = true;
  }
  
  return pressed;
}

void heartbeatWait(bool buttonPressed) {

  if (buttonPressed == true) {
    Serial.println("Button pressed waiting");
    for (int index = 0; index < HEARTBEAT_TIME_MINUTES; index++) {
      for (int iteration = 0; iteration < 30; iteration++) {
        buttonPressed = checkButtonPressed();
        if (buttonPressed == false) {
          break;
        }
        delay(TIME_SECOND * 2);
      }
      
      Serial.println(".");
      if (buttonPressed == false) {
        break;
      }
    }
  }
  else {
   Serial.println("Button not pressed waiting");
   for (int index = 0; index < HEARTBEAT_TIME_MINUTES; index++) {
    delay(TIME_MINUTE); 
    Serial.println(".");
   } 
  }  
}

void encodeButtonPressed(char (*status)[MAX_MESSAGE_LENGTH], bool pressed) {
  char character = 'N';

  if (pressed == true) {
    character = 'B';
  }

  (*status)[BUTTON_INDEX] = character;
}

void checkTemperature(char (*status)[MAX_MESSAGE_LENGTH]) {
    float temperature = temperatureSensor.readTemperatureCelsius(sensors);
    char character = temperatureSensor.encodeTemperatureToChar(temperature);

    (*status)[TEMP_INDEX] = character;
}

void checkVibration(char (*status)[MAX_MESSAGE_LENGTH]) {
    float voltage = -1.00;
    voltage = vibrationSensor.getVibrationReading();
    (*status)[VIBRATION_INDEX] = vibrationSensor.convertReadingToChar(voltage);  
}

int sendMessage(char (*status)[MAX_MESSAGE_LENGTH]) {
  int messageStatus = sigfox.sendMessage(*status);
  return messageStatus;
}

void setup() {
  buttonPress.begin(BUTTON_PIN);
  temperatureSensor.begin(TEMP_PIN, sensors);
  vibrationSensor.begin(VIBRATION_PIN);
  Serial.begin(9600);
  SigFox.debug();
}

void loop() {

  delay(TIME_SECOND * 10);
  char status[MAX_MESSAGE_LENGTH];

  int index;
  for (index = 0; index < MAX_MESSAGE_LENGTH; index++) {
    status[index] = '\0';
  }

  Serial.println("Checking button");
  bool buttonPressed = checkButtonPressed();
  encodeButtonPressed(&status, buttonPressed);

  Serial.println("Checking Temperature");
  checkTemperature(&status);

  Serial.println("Checking Vibration");
  checkVibration(&status);

  Serial.println("Sending Message");
  int messageSent = sendMessage(&status);
  if (messageSent > 0) {
    // If not sent, try again.
    delay(TIME_SECOND * 10);
    sendMessage(&status);
  }

  heartbeatWait(buttonPressed);
}
