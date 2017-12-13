// Make sure to remove wire in pin 10 to
// successfully upload the code to the Arduino.

#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

SoftwareSerial mySoftwareSerial(2, 3); // RX, TX

// Alias mp3 library as Player
DFRobotDFPlayerMini Player;

const int blueLedPin1 = 5;
const int blueLedPin2 = 6;
const int blueLedPin3 = 9;
const int blueLedPin4 = 10;

const int redLedPin1 = 7;
const int redLedPin2 = 8;
const int redLedPin3 = 11;
const int redLedPin4 = 13;

// Switch reads from pin 12
const int onOffSwitchPin = 4;
const int shelfSwitchPin = 12;

// Variables will change:
int shelfSwitchState;             // the current reading from the input pin
int lastShelfSwitchState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

/*
 * Track 1: "A swing and a drive, to deep left, awaaay back, gone!"
 * Track 2: "Swiiing and a miss, ballgame!"
 * Track 3: "How about that?!"
 * Track 4: "Strike three called!"
 * Track 5: "Swung on and belted, to deep left, awaaay back, gone! To the home run porch!"
 * Track 6: "And we're underway at the corner of Carnegie and Ontario"
 * Track 7: "Swiiing and a miss!"
 * Track 8: "Ballgame!"
 * Track 9: "Swing and a high drive, deep right center, there she goes! A game winner!"
 */

int trackIndex;
int playbackComplete = 0;
unsigned long playbackStartedAt = 0;
unsigned long playbackTime = 0;

void setup() {
  // initialize the switch pin as an input
  pinMode(shelfSwitchPin, INPUT);
  digitalWrite(shelfSwitchPin, HIGH);

  pinMode(onOffSwitchPin, INPUT);
  digitalWrite(onOffSwitchPin, HIGH);

  // initialize the led pin as an output
  pinMode(blueLedPin1, OUTPUT);
  pinMode(blueLedPin2, OUTPUT);
  pinMode(blueLedPin3, OUTPUT);
  pinMode(blueLedPin4, OUTPUT);

  pinMode(redLedPin1, OUTPUT);
  pinMode(redLedPin2, OUTPUT);
  pinMode(redLedPin3, OUTPUT);
  pinMode(redLedPin4, OUTPUT);

  // Choose a random seed to base the track randomization
  randomSeed(analogRead(0));

  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!Player.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  Player.volume(30);  //Set volume value. From 0 to 30
}

void loop() {
  // read the state of the switch on pin 12 into a local variable:
  int reading = digitalRead(shelfSwitchPin);

  // If player is playing, set playback time.
  if (playbackStartedAt > 0) {
    playbackTime = millis() - playbackStartedAt;

    setTrackSpecificLighting(trackIndex, playbackTime);
  }

  // check to see if a beer can was just dispensed.
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastShelfSwitchState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the shelf switch state has changed:
    if (reading != shelfSwitchState) {
      shelfSwitchState = reading;
      int onOffSwitchState = digitalRead(onOffSwitchPin);

      Serial.print(F("Power switch is "));
      Serial.println(onOffSwitchState);
      
      // only trigger the audio if the new button state is HIGH
      if (shelfSwitchState == HIGH) {

        if (onOffSwitchState == HIGH) {
          // select a random track from 1-9
          trackIndex = random(1, 10);
          Player.play(trackIndex);
          playbackStartedAt = millis();
          playbackComplete = 0;
  
          Serial.print(F("Now playing: Track "));
          Serial.println(trackIndex);
  
          // Turn the lights on
          // fade in from min to max in increments of 5 points:
          for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
            // sets the value (range from 0 to 255):
            analogWrite(blueLedPin1, fadeValue);
            analogWrite(blueLedPin2, fadeValue);
            analogWrite(blueLedPin3, fadeValue);
            analogWrite(blueLedPin4, fadeValue);
            // wait for 30 milliseconds to see the dimming effect
            delay(30);
          }
        } else {
          // Lights only

          // Turn the lights on
          // fade in from min to max in increments of 5 points:
          for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
            // sets the value (range from 0 to 255):
            analogWrite(blueLedPin1, fadeValue);
            analogWrite(blueLedPin2, fadeValue);
            analogWrite(blueLedPin3, fadeValue);
            analogWrite(blueLedPin4, fadeValue);
            // wait for 30 milliseconds to see the dimming effect
            delay(30);
          }

          strobeRedLeds();

          delay(2000);

          // fade out from max to min in increments of 5 points:
          for (int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) {
            // sets the value (range from 0 to 255):
            analogWrite(blueLedPin1, fadeValue);
            analogWrite(blueLedPin2, fadeValue);
            analogWrite(blueLedPin3, fadeValue);
            analogWrite(blueLedPin4, fadeValue);
    
            // wait for 30 milliseconds to see the dimming effect
            delay(30);
          }

        } // endif

      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastShelfSwitchState:
  lastShelfSwitchState = reading;

  if (Player.available()) {
    printDetail(Player.readType(), Player.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}

// No special lighting for track 6, 7, 8
void setTrackSpecificLighting(int trackIndex, unsigned long playbackTime) {

  switch(trackIndex) {
    case 1:
      if (playbackTime > 4400) {
        Serial.println(F("Gone!")); // good
        strobeRedLeds();
      }
    break;
    case 2:
      if (playbackTime > 1700) {
        Serial.println(F("Ballgame!")); //good
        strobeRedLeds();
      }
    break;
    case 3:
      strobeRedLeds();
      // flicker whole time
    break;
    case 4:
      if (playbackTime > 1700) {
        Serial.println(F("Called!")); //goood
        strobeRedLeds();
      }
    break;
    case 5:
      if (playbackTime > 5000) {
        Serial.println(F("Gone!"));
        strobeRedLeds();
      }
    break;
    case 9:
      if (playbackTime > 3500) {
        Serial.println(F("There she goes!")); // good
        strobeRedLeds();
      }
    break;
    default:
    break;
  }

}

void strobeRedLeds() {
  for(int i = 0; i < 10; i += 1) {
    digitalWrite(redLedPin1, !digitalRead(redLedPin1));
    delay(50);
    digitalWrite(redLedPin2, !digitalRead(redLedPin2));
    delay(50);
    digitalWrite(redLedPin3, !digitalRead(redLedPin3));
    delay(50);
    digitalWrite(redLedPin4, !digitalRead(redLedPin4));
    delay(50);
  }
}

void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));

      // Fixes issue with this callback being called twice consecutively.
      if (playbackComplete == 0) {
        // fade out from max to min in increments of 5 points:
        for (int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) {
          // sets the value (range from 0 to 255):
          analogWrite(blueLedPin1, fadeValue);
          analogWrite(blueLedPin2, fadeValue);
          analogWrite(blueLedPin3, fadeValue);
          analogWrite(blueLedPin4, fadeValue);
  
          // wait for 30 milliseconds to see the dimming effect
          delay(30);
        }
      }

      playbackStartedAt = 0;
      playbackComplete = 1;
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

}
