
#include <EEPROM.h>
#include <Bounce2.h>
#define FRONT_BUTTON 6
#define HALL_INT 2
#define CHIM_LED 4
#define RGB_PIN 8
#define MOTOR_ON A0
#define MOTOR_DIRECTION A1
Bounce debouncer = Bounce(); 

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2, RGB_PIN, NEO_GRB + NEO_KHZ800);

boolean pinState; //pin state of photo interrupt.
boolean previousState = 0; //previous state of interrupt, used for calculating stop station.
boolean RED = false;


boolean driveDirection = true;

int ledPWM = 255;
int ledFadeAmount = 1;
int decreaseSpeed = 5;

void setup() {
  EEPROM.write(0, !EEPROM.read(0));
  
  pinMode(HALL_INT,INPUT_PULLUP);
  pinMode(FRONT_BUTTON,INPUT_PULLUP);
  pinMode(MOTOR_ON,OUTPUT);
  pinMode(MOTOR_DIRECTION,OUTPUT);
  pinMode(CHIM_LED, OUTPUT);
  pinMode(RGB_PIN, OUTPUT);
  digitalWrite(RGB_PIN, LOW);
  digitalWrite(MOTOR_ON, LOW); // Speed = 0 - 255. Low = 0, high = 255
  digitalWrite(MOTOR_DIRECTION, HIGH); //forward = high , reverse = low
  digitalWrite(CHIM_LED, HIGH);
  debouncer.attach(FRONT_BUTTON);
  debouncer.interval(20); // interval in ms

  pixels.begin();
  pixels.show();

  Serial.begin(9600);
  //Serial.println("setup end");
}


void loop() {
  //Serial.println("loop started");
  debouncer.update();
  if ( debouncer.fell() ) {
    //Serial.println("start train called");   
    startTrain();
    //Serial.println("Stop train / soft stop");      
    stopTrain();
    //softStopTrain();
    //Serial.println("Pick shot glass");      
    pickShotGlass();

    delay(10000);
    turnOffShotsLEDS();
  }
}  
  
void startTrain()  {
  int magnetsTillStop = random(1, 13);
  int i = 0;
  // wait 3 seconds to clear fingers from train
  ledFlash(3, 500, 500);
    
  drive(driveDirection);
  unsigned long value;  
  //Serial.println("magnetsToNextStop");
  //Serial.println(magnetsTillStop);
  while(i<magnetsTillStop) {
    value = hallSensor();
    if( value == 1) {
      i++;
      ledFlash(1, 100, 100);
      //Serial.println("playerCount:");
      //Serial.println(i);
    }    
  }
}

// DRIVE FUNCTION 0 = forward, 1 = reverse.
void drive(boolean driveDirection)  {
  if (driveDirection == 0) {
    digitalWrite(MOTOR_DIRECTION, HIGH);
    digitalWrite(MOTOR_ON, HIGH);
  } 
  else if (driveDirection == 1)  {
    digitalWrite(MOTOR_ON, HIGH);
    digitalWrite(MOTOR_DIRECTION, LOW);    
  }
}

void stopTrain() {
  digitalWrite(MOTOR_ON, LOW);  
  delay(500);   
}

void softStopTrain() {
  int currentSpeed = 255;    
  //Serial.println("slow stop called");
  while(currentSpeed > 0) {
    currentSpeed = currentSpeed - decreaseSpeed;

    if (currentSpeed < 0) {
      currentSpeed = 0;    
    }
    
    analogWrite(MOTOR_ON, currentSpeed);    
    delay(300);
  }
}

unsigned long hallSensor() {
  pinState = digitalRead(HALL_INT);
  if(pinState == LOW && previousState == 0)  {
    previousState = 1;
    return 0;
  }
  else if(pinState == HIGH && previousState == 1) {    
    previousState = 0; 
    Serial.println("Hall Sensor sends 1");   
    return 1;
  }
  else return 0;
}

void ledFlash(int flashTimes, int flashLength, int delayLength) {
    for(int i=0; i < flashTimes; i++) {
      digitalWrite(CHIM_LED, LOW);
      delay(flashLength);
      digitalWrite(CHIM_LED, HIGH);
      delay(delayLength);
    }
}

void pickShotGlass()  {
  int pickShotGlass = random(12);
  boolean giveTake = random(2);
  int shotGlass; //1 = first, 2 = second, 3 = both

  //Serial.println("Random: Pick shotglass");
  //Serial.println(pickShotGlass);

  switch(pickShotGlass) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      shotGlass = 1;
      break;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      shotGlass = 2;
      break;
    case 11:
    case 12:
      shotGlass = 3;
      break;
    }

  //Serial.println("Shot glas valgt:");
  //Serial.println(shotGlass);

  //Serial.println("Give/take:");
  //Serial.println(giveTake);

  SpinRainBowShotColors();
  selectGiveOrTake(giveTake, shotGlass);
}

void SpinRainBowShotColors() {
  int rainBowCycles = random(10, 20);
  for(int i=0; i<rainBowCycles; i++)  {
      theaterChaseRainbow(0);
  }
  turnOffShotsLEDS();
}

void selectGiveOrTake(int giveTake, int shotGlass){
  switch(shotGlass) {
    case 1: // first shot glass
      if (giveTake == RED) {
        pixels.setPixelColor(0, 0, 255, 0);
      }
      else {
        pixels.setPixelColor(0, 255, 0, 0);
      }
    break;
    case 2: // second shot glass
      if (giveTake == RED) {
        pixels.setPixelColor(1, 0, 255, 0);
      }
      else {
        pixels.setPixelColor(1, 255, 0, 0);
      }
    break;
    case 3: // both shot glasses
    if (giveTake == RED) {
      pixels.setPixelColor(0, 0, 255, 0);
      pixels.setPixelColor(1, 0, 255, 0);
      }
    else {
      pixels.setPixelColor(0, 255, 0, 0);
      pixels.setPixelColor(1, 255, 0, 0);      
      }  
    break;
    }
    pixels.show();
}

void turnOffShotsLEDS(){
  pixels.setPixelColor(0, 0);
  pixels.setPixelColor(1, 0);
  pixels.show();
}


void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
