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
   Serial.print(".");
   delay(TIME_MINUTE); 
  }
  Serial.println("");
  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SigFox.debug();
  buttonPress.begin(BUTTON_PIN);
  temperatureSensor.begin(TEMP_PIN, sensors);
  vibrationSensor.begin(VIBRATION_PIN);
}

bool checkButtonPressed(char (*status)[MAX_MESSAGE_LENGTH]) {
  bool pressed = false;
  char character = 'N';
  int buttonPressed = buttonPress.isPressed();
  
  if (buttonPressed == 1) 
  {
    character = 'B';
    pressed = true;
  }

  (*status)[BUTTON_INDEX] = character;
  
  return pressed;
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10 * TIME_SECOND);

  char status[MAX_MESSAGE_LENGTH];

  int index;
  for (index = 0; index < MAX_MESSAGE_LENGTH; index++) {
    status[index] = '\0';
  }
  
  bool buttonPressed = checkButtonPressed(&status);

  {
    float temperature = temperatureSensor.readTemperatureCelsius(sensors);
    char character = temperatureSensor.encodeTemperatureToChar(temperature);

    status[TEMP_INDEX] = character;
    Serial.print("Temperature (Celsius) is: ");
    Serial.println(temperature);
  }

  {
    float voltage = -1.00;
    voltage = vibrationSensor.getVibrationReading();
    status[VIBRATION_INDEX] = vibrationSensor.convertReadingToChar(voltage);
    Serial.print("Vibration reading: ");
    Serial.println(voltage);
  }

  {
    int messageSent = sigfox.sendMessage(status);
    if (messageSent > 0) {
      Serial.println("Message was not sent");
    } else {
      Serial.println("Message sent");
    }
  }

  heartbeatWait();
}
