void move(int speed, int directionTo) { // General Move with Direction (no target location). Speed needs to be 0-255, Direction 1 or 2

  direction = directionTo; // Stores the direction in a global parameter

  if (motorDisabled == 1) {
    Serial.println("Motor disabled, can't move");
    return;
  }

  motionDone = 0;

  if (directionTo == 1) { // clockwise
    digitalWrite(CWPin, HIGH);
    digitalWrite(CCWPin, LOW);
  } else if (directionTo == 2) { // counter-clockwise
    digitalWrite(CWPin, LOW);
    digitalWrite(CCWPin, HIGH);
  } else {
    digitalWrite(CWPin, LOW);
    digitalWrite(CCWPin, LOW);
    Serial.println("Bad Direction, use 1 for CW and 2 for CCW");
    return;
  }

  analogWrite(motorSpeedPin, speed);
  Serial.println("Moving with speed " + speed);
}

void moveTo(int speed, int directionTo, long goToPosition) { // Move to a specific location ******

  direction = directionTo;

  if (motorDisabled == 1) {
    Serial.println("Motor disabled, can't move");
    return;
  }

  // print the command
  Serial.print("Command Received: Speed: ");
  Serial.print(speed);
  Serial.print("\t Direction: ");
  Serial.print(directionTo);
  Serial.print("\t goToPosition: ");
  Serial.println(goToPosition);

  motionDone = 0; // set the flag for movement (not done is zero means it's moving)

  if (directionTo == 1) { // clockwise
    digitalWrite(CWPin, HIGH);
    digitalWrite(CCWPin, LOW);
  } else if (directionTo == 2) { // counter-clockwise
    digitalWrite(CWPin, LOW);
    digitalWrite(CCWPin, HIGH);
  } else {
    digitalWrite(CWPin, LOW);
    digitalWrite(CCWPin, LOW);
    Serial.println("Bad Direction, use 1 for CW and 2 for CCW");
    return;
  }

  analogWrite(motorSpeedPin, speed); //move the motor
  Serial.println("Moving"); // DEBUG

}

void stopMotor() {
  Serial.println("Done Moving");
  analogWrite(motorSpeedPin, 0);
  motionDone = 1;
}

void checkStop() {
  if (((newPosition + 40 >= cmdPosition) && (newPosition - 40 <= cmdPosition )) && (motionDone == 0)) {
    Serial.println("Stop Condition"); //DEBUG
    stopMotor();
  }
  checkMax();
}

void checkMax() {
  if ((newPosition + 40 >= maxPosition) && (motionDone == 0) && (direction == 1)) {
    motionDone = 0;
    Serial.println("Reached Max");
    cmdPosition = 0;
    moveTo(100, 2, cmdPosition); //move back to 0

  }
}

void findEdges () { //This function finds the edges for the motor movements - Max and Min
  // uses a limit switch (or a hall magnet switch)

  Serial.println("FINDING EDGES");
  delay(3000);
  while ((analogRead(limitSwPin) <= magnetHigh) && (analogRead(limitSwPin) >= magnetLow)) { // numbers might need adjusting based on analog reads of hall sensor
    move(150, 2);
    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      Serial.print ("enc position: "); // DEBUG - Disable eventually
      Serial.println(newPosition); // DEBUG - Disable eventually
    }
  }
  stopMotor();

  myEnc.write(0); // writes 0 to the encoder location
  Serial.println("Limit Switch Activated - MIN"); // DEBUG
  Serial.print("New Limit: "); // DEBUG
  Serial.println(myEnc.read());

  move(100, 1);
  delay(2000); //wait a few seconds


  while ((analogRead(limitSwPin) <= magnetHigh) && (analogRead(limitSwPin) >= magnetLow)) { // numbers might need adjusting based on analog reads of hall sensor
    move(150, 1);
    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      Serial.print ("enc position: "); // DEBUG - Disable eventually
      Serial.println(newPosition); // DEBUG - Disable eventually
    }
  }
  maxPosition = newPosition;
  stopMotor();
  Serial.print("MAX Position: ");
  Serial.println(maxPosition); // DEBUG - Disable eventually
  delay(3000);
  cmdPosition = 0;
  Serial.println("Moving to 0 point");
  moveTo(100, 2, cmdPosition); // Move back to 0


}


