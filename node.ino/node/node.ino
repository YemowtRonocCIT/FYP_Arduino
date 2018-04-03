#include <Arduino.h>
#include <SigFox.h>
#include <ButtonPress.h>
#include <TemperatureSensor.h>
#include <VibrationSensor.h>
#include <SigfoxMessage.h>


const int TIME_SECOND = 1000;
const int TIME_MINUTE = TIME_SECOND * 60;
const int TIME_HOUR = TIME_MINUTE * 60;


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

void heartbeatWait() {

  for (int index = 0; index < 10; index++) {
   delay(TIME_MINUTE); 
  }
  
}

void setup() {
  buttonPress.begin(BUTTON_PIN);
  temperatureSensor.begin(TEMP_PIN, sensors);
  vibrationSensor.begin(VIBRATION_PIN);
}

bool checkButtonPressed() {
  bool pressed = false;
  int buttonPressed = buttonPress.isPressed();
  
  if (buttonPressed == 1) 
  {
    pressed = true;
  }
  
  return pressed;
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

void loop() {
  char status[MAX_MESSAGE_LENGTH];

  int index;
  for (index = 0; index < MAX_MESSAGE_LENGTH; index++) {
    status[index] = '\0';
  }
  
  bool buttonPressed = checkButtonPressed();
  encodeButtonPressed(&status, buttonPressed);

  checkTemperature(&status);

  checkVibration(&status);

  int messageSent = sendMessage(&status);
  if (messageSent > 0) {
    // If not sent, try again.
    sendMessage(&status);
  }

  heartbeatWait();
}
