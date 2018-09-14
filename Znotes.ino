/***** A02K MAGNETIC HALL EFFECT SENSOR *****:
It's an Analog sensor. Averages around 500. Depending on the pole of the magnet facing it, it will either go down or up.
http://bit.ly/2z8tUJ4

Wiring:
curved side facing facing towards you, left to right:
VCC
GND
Signal
1-10K resistor between VCC and signal.


***** MOTOR NOTES *****
Slowest I can move it seems to be about 70 PWM.

Wiring for the Pololu Motor driver
from right to left:
GND, V+, Motor A (gnd), Motor B (VCC)

// *** MOTOR CONTROL PINS ***
#define enablePin 4 // Connect to EN - Send High or Low to enabled and disable the motor
#define motorSpeedPin 5 // Connect to D2 on the Pololu MD05A (Needs to be PWM pin)
#define CWPin 6 // Connect to IN1 - Turning HIGH will change the motor direction to ClockWise
#define CCWPin 7 // Connect to IN2 - Turning HIGH will change the motor direction to Counter ClockWise

*/
