
// ***** Timer *****
#include <SimpleTimer.h>
SimpleTimer timer; // the timer object

// timer library - https://github.com/zomerfeld/SimpleTimerArduino

// ***** RTC *****
#include <RTClib.h> //library from https://github.com/adafruit/RTClib
RTC_DS1307 rtc;
DateTime now;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int nowHour = -1;
int nowMinute = -1;
int nowSecond = -1;
int lastHour = -1;
int lastMinute = -1;
int lastSecond = -1;

// Wiring: 5V to 5V, GND to GND, SCL to A5 (on Uno, changes by controller), SDA to A4 (on Uno)
// Wiring: https://screencast.com/t/50Cv0fAUM7w5


// ***** Encoder http://www.pjrc.com/teensy/td_libs_Encoder.html
// If you define ENCODER_DO_NOT_USE_INTERRUPTS *before* including
// Encoder, the library will never use interrupts.  This is mainly
// useful to reduce the size of the library when you are using it
// with pins that do not support interrupts.  Without interrupts,
// your program must call the read() function rapidly, or risk
// missing changes in position.
//#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#include <Bounce2.h>

//   Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
//   Make sure to pull UP those pins up with a 1K resistor (though the code seems to use the INPUT_PULLUP)

Encoder myEnc(2, 3); //on Uno, the pins with interrupt capability are 2 and 3 (https://www.arduino.cc/en/Reference/attachInterrupt)

// *** MOTOR CONTROL PINS ***
#define enablePin 4 // Connect to EN - Send High or Low to enabled and disable the motor
#define motorSpeedPin 5 // Connect to D2 on the Pololu MD05A (Needs to be PWM pin)
#define CWPin 6 // Connect to IN1 - Turning HIGH will change the motor direction to ClockWise
#define CCWPin 7 // Connect to IN2 - Turning HIGH will change the motor direction to Counter ClockWise
// (flip the A/B wires if the direction is reversed)

#define fwdButton 8 //Button to move motor forward
#define backButton 9 //Button to move motor backward
#define debugLED 13 // Debug LED


// *** LIMIT SWITCH  ***
#define limitSwPin A0 // The pin for the limit switch.  
// For a regular switch, set as INPUT_PULLUP and connect the switch to GND and look for LOW for trigger. (https://www.arduino.cc/en/Tutorial/DigitalInputPullup)
int magnetHigh = 520; // high range for magnet detection (460+578?)
int magnetLow = 440;

// Initiate a Bounce object: //needed for digital switch
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();

// **************************

// *** SERIAL VARIABLES ***
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
// **************************

// *** CLOCK VARIABLES ***
bool motorDisabled = 0; //To disable the motor passed limits
long maxPosition = -1;
long cmdPosition = 200; // Where we're aiming the motor to go
bool motionDone = 1; // If the clock's in motion or not
long distanceMinute = 3000; // CHANGE - How much we need to move for one minute passing
long distance5Second = 250; // CHANGE - How much we need to move for 5 seconds passing
int direction; // globally stores the direction of the moveTo Commands.

// **************************

// *** ENCODER VARIABLES ***
long oldPosition  = -999;
long newPosition;


// ************************** SETUP **************************

void setup() {

  // ***** STARTS SERIAL & RTC *****
  Serial.begin(250000);
  Serial.println("Rachel's Clock");

  Serial.println("***STARTING RTC***");

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }

  inputString.reserve(200);   // reserve 200 bytes for the inputString


  // Setting Timers
  timer.setInterval(4999, showTime); // This will display the time every 5 seconds on serial. Disable if needed.
  //  timer.setInterval(1000, minuteMove); // This will move the motor every minute. Not needed currently
//  timer.setInterval(5000, fiveSecMove); //moves the motor every 5 second forward. Should not be enabled by default


  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time,
  // rtc.adjust(DateTime(2017, 6, 2, 15, 29, 0));


  // ***** PIN SETUP *****
  pinMode(enablePin, OUTPUT);
  pinMode(motorSpeedPin, OUTPUT);
  pinMode(CWPin, OUTPUT);
  pinMode(CCWPin, OUTPUT);
  pinMode(limitSwPin, INPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(fwdButton, INPUT_PULLUP);
  pinMode(backButton, INPUT_PULLUP);
  pinMode(debugLED, OUTPUT);
  

  // After setting up the limit SW, setup the Bounce instance (only needed for digital switch :
  //  debouncer.attach(limitSwPin);
  //  debouncer.interval(90); // 90 seemed to work fast enough. Test and modify if needed

  digitalWrite(enablePin, HIGH); // Turns the motor on

  debouncer1.attach(fwdButton);
  debouncer2.attach(backButton);

  debouncer1.interval(90); // 90 seemed to work fast enough. Test and modify if needed
  debouncer2.interval(90); // 90 seemed to work fast enough. Test and modify if needed

  findEdges();

}

// ************************** LOOP **************************

void loop() {
  // ***** UPDATE TIME *****
  timer.run();



  // ***** READ ENCODER *****
  newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.print ("encoder position: "); // DEBUG - Disable eventually
    Serial.println(newPosition); // DEBUG - Disable eventually
  }
  // **************************

  // **** CHECK SWITCHes ****
  debouncer1.update();
  debouncer2.update();
//  if (analogRead(limitSwPin) > 519) { // OLD WAY - might change the number and / or the direction depend o magent pul
  if ((analogRead(limitSwPin) >= magnetHigh) || (analogRead(limitSwPin) <= magnetLow)) { // numbers might need adjusting based on analog reads of hall sensor

    //    myEnc.write(0); // writes 0 to the encoder location
    Serial.println("Limit Switch Activated"); // DEBUG
    digitalWrite(debugLED, HIGH); //turn on debug led
  } else {
    digitalWrite(debugLED, LOW); //turn off debug LED
  }

  // **** Serial handling  ****
  if (stringComplete) {
    Serial.println(inputString);

    // command to manual move: moveTo(X,Y,Z)
    // X = Speed (0-255), Y direction (1or2), Z location )
    // moveTo(120,1,3000)

    if (inputString.startsWith("moveTo")) { // if the string begins with moveTo - Case Sensitive!
      int commaPosition1 = inputString.indexOf(','); //gets the location of the first comma
      String cmdSpeedSt = inputString.substring(7, commaPosition1); // cuts the string between the 7th char and the first comma
      int cmdSpeed = cmdSpeedSt.toInt(); // casts the SPEED it into an int
      Serial.print("cmdSpeed: ");
      Serial.println(cmdSpeed);

      String cmdDirectionSt = inputString.substring(commaPosition1 + 1, commaPosition1 + 2); // gets the direction
      int cmdDirection = cmdDirectionSt.toInt(); // casts the DIRECTION into an int
      Serial.print("cmdDirection: ");
      Serial.println(cmdDirection);

      //String cmdRest = inputString.substring(commaPosition1 + 2);
      //      Serial.print("cmdRest: "); // DEBUG
      //      Serial.println(cmdRest); // DEBUG

      int commaPosition2 = inputString.lastIndexOf(','); // gets the location of the last comma
      int parePosition = inputString.indexOf(')'); //gets the location of the closing parentheses
      //      Serial.print("parePosition: "); Serial.println(parePosition); // DEBUG

      String cmdPositionSt = inputString.substring(commaPosition2 + 1, parePosition); // cuts the string between the last comma to the parentheses
      cmdPosition = cmdPositionSt.toInt(); //casts the TARGET POSITION into a global variable (long actually, which will accept an int)
      Serial.print("cmdPosition: ");
      Serial.println(cmdPosition);

      // engage the motor via the moveTo Function
      moveTo(cmdSpeed, cmdDirection, cmdPosition);

    }

    // clear the string:
    inputString = "";
    stringComplete = false;
  }

  // ***** MOTOR STOPPING *****
  checkStop();

  // ***** MANUAL CONTROL BUTTONS *****
  if (debouncer1.fell()) { //if the button went to low (set to pullup)
    digitalWrite(CWPin, HIGH);
    digitalWrite(CCWPin, LOW);
    analogWrite(motorSpeedPin, 125);
    Serial.println("moving manually");
  }

  if (debouncer2.fell()) {
    digitalWrite(CWPin, LOW);
    digitalWrite(CCWPin, HIGH);
    analogWrite(motorSpeedPin, 100);
    Serial.println("moving manually");
  }

  if (debouncer1.rose()) { //when the buttons are not pressed anymore 
    stopMotor();
  }

  if (debouncer2.rose()) {
    stopMotor();
  }

}



