int limit = 20; //what distance should the sensors react to



int a; //the integer given to kenneths base code

#define trigPinFwd 7 //Pin that triggers ultrasonic signal
#define echoPinFwd 6 //Pin that receives returned signal

#define trigPinL 9 //trig pin on left sensor
#define echoPinL 8

#define trigPinR 11 //trig pin on rigth sensor
#define echoPinR 10 


//functions
int leftSensor() {
  long durationL, distanceL;
  digitalWrite(trigPinL, LOW);
  delay(2);
  digitalWrite(trigPinL, HIGH);
  delay(10);
  digitalWrite(trigPinL, LOW);
  durationL = pulseIn(echoPinL, HIGH);
  distanceL = (durationL/2) /29.1;
  return distanceL;
}

int rightSensor() {
  long durationR, distanceR;
  digitalWrite(trigPinR, LOW);
  delay(2);
  digitalWrite(trigPinR, HIGH);
  delay(10);
  digitalWrite(trigPinR, LOW);
  durationR = pulseIn(echoPinR, HIGH);
  distanceR = (durationR/2) /29.1;
  return distanceR;
}

int frontSensor() {
  long durationFwd, distanceFwd;
  digitalWrite(trigPinFwd, LOW); //sending nothing
  delay(2); 
  digitalWrite(trigPinFwd, HIGH); //sending high signal
  delay(10);
  digitalWrite(trigPinFwd,LOW); //sending nothing
  durationFwd = pulseIn(echoPinFwd,HIGH); //measures duration of the reveiced high signal on the echo pin
  //calculate distance 
  distanceFwd = (durationFwd/2) / 29.1; // gives distance in cm
  return distanceFwd;
}

boolean buttonPushed() { //dummy function. Should implement a button and when the button is pushed, this function should return true
  return true;
}

boolean objectInFront () {
  return (frontSensor() < limit); //returns true if distanse to sensor is below limit
}

boolean objectLeft() {
  return (leftSensor() < limit); //returns true if distanse to sensor is below limit
}

boolean objectRight() {
  return(rightSensor() < limit); //returns true if distanse to sensor is below limit
}

boolean backUp() {
  return(objectRight() && objectLeft() && objectInFront()); // return true if all objects are below limit 
}

boolean noRestrictions() {
  return(!objectRight() && !objectLeft() && !objectInFront()); //returns true if there are no objects in front of any sensor
}

boolean sidesBlocked() {
  return (objectRight() && objectLeft());
}



enum STATE {
  STILL, //Case 0: Button is not pushed
  FREE, //Case 1 : No restrictions
  FB_RB_TURN_LEFT, //Case 2: FrontBlocked, RightBlocked, turn left
  FB_LB_TURN_RIGHT, //Case 3: FrontBlocked, LeftBlocked, turn right
  FC_LC_RB, //Case 4: FrontClear, LeftClear, RigthBlocked
  FC_LB_RC, //Case 5: FrontClear, RightClear, LeftBlocked
  FC_LB_RB, //Case 6: FrontClear, LeftBlocked, RightBlocked
  FB_LB_RB, //Case 7: FrontBlocked, LeftBlocked, RigthBlocked (the back up scenario)
  
};

enum US_STATE {
  NORMAL, //NormalCase
  SPECIAL1, //SpecialCase step 1
  SPECIAL2, //SpecialCase step 2
  SPECIAL2_1L,
  SPECIAL2_1R,
  SPECIAL3_L, //SpecialCase step 3
  SPECIAL3_R,
  SPECIAL4_L,
  SPECIAL4_R
};

US_STATE USstate = NORMAL; //assume normal conditions during start up
STATE currentState = FREE; //assume free state during start up




void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  pinMode(trigPinFwd, OUTPUT);
  pinMode(echoPinFwd, INPUT); 

  pinMode(trigPinL, OUTPUT);
  pinMode(echoPinL, INPUT);

  pinMode(trigPinR, OUTPUT);
  pinMode(echoPinR, INPUT);
  Serial.println("Initializing the obstacleAvoidance function");
}

void loop() {
  // put your main code here, to run repeatedly:
  obstacleAvoidance();
  delay(5000);

}

void obstacleAvoidance() { //void obstacleAvoidance(int& currentState, int& USstate)




  
  switch(USstate) { //switches between normal and special

    case NORMAL: //in case of normal
    if(noRestrictions()) {
      currentState = FREE;
      Serial.println("Restrictions: None. State: Normal.");
      break;
    }
    else if(objectInFront()) { //front is blocked
      if(sidesBlocked()) {
        currentState = FB_LB_RB; //THIS CASE SHOULD HAVE ITS OWN FUNCTION TO SOLVE
        Serial.println("Restrictions: All sides blocked. State: Entering Special State.");
        //break;
      }
      else if(objectLeft() && !objectRight()){ //front is blocked, left is blocked, right is free
        currentState = FB_LB_TURN_RIGHT;
        //USstate = NORMAL; 
        Serial.println("Restrictions: Front blocked & left blocked. State: Normal.");
        break;
      }
      else if(!objectLeft() && objectRight()) { //front is blocked, right is blocked, left is free
        currentState = FB_RB_TURN_LEFT;
        Serial.println("Restrictions: Front blocked & right blocked. State: Normal.");
        break; 
      }
      else { //no side restrictions but object in front
        currentState = FB_RB_TURN_LEFT;
        Serial.println("Restrictions: Front blocked. Both sides clear. By default go left. State: Normal");
      }
    }
    else { //no object in front but restrictions 
      if(objectRight() && !objectLeft()) { //restriction: object to the right, left free
        currentState = FC_LC_RB;
        Serial.println("Restrictions: Right is blocked. State: Normal.");
        break;
      }
      else if(objectLeft() && !objectRight()) { //restriction: object to left, right free
        currentState = FC_LB_RC;
        Serial.println("Restrictions: Left is blocked. State: Normal.");
        break;
      }
      else { //front is clear but both sides blocked 
        currentState = FC_LB_RB;
        Serial.println("Restrictions: Both sides blocked (front is clear). State: Normal.");
        break;
      }
      }
      
      if (currentState == FB_LB_RB) { //if the currentState is FrontBlocked, LeftBlocked, RightBlocked and the USstate is NORMAL, enter special case. Start with special case 1
      USstate = SPECIAL1;
      case SPECIAL1:
      //get out of step 1 
      if (!objectLeft() || !objectRight()){ //if one side clears, move to the next step
      USstate = SPECIAL2;
      Serial.println("Restrictions: One side has been cleared. State: Entering SPECIAL2.");
      break;
      }
      else {
        currentState = FB_LB_RB;
        USstate = SPECIAL1; //if none of the sides have been cleared yet, keep backing up. do this by staying in special1.
        Serial.println("Restrictions: Both sides blocked, will continue to back up. Staying in state: SPECIAL1.");
        break;
      }
      //step 2, act as if front is blocked
      case SPECIAL2: //treat as if there is an object in front and one of the sides have been cleared
      if(!objectLeft()) { //if its the left side that is cleared
        currentState = FB_RB_TURN_LEFT;
        if(objectInFront()) {
          USstate = SPECIAL2_1L; //Enter case special 2_1Left
          Serial.println("Restrictions: Left has been cleared. Right side is blocked. State: Entering SPECIAL2_1R.");
        }
       else{
        USstate = SPECIAL2;
        Serial.println("State: In SPECIAL2, will keep turning left so I can enter SPECIAL2_1R");
        
       }
       break;
      }
      else if(!objectRight()) { //if its the right side that is cleared
        currentState = FB_LB_TURN_RIGHT;
        if(objectInFront()) {
          USstate = SPECIAL2_1R; //Enter case special2_1RIGHT
            Serial.println("Restrictions: Left side is blocked. State: Entering SPECIAL2_1L.");
            
        }
       else{
        USstate = SPECIAL2;
        Serial.println("State: In SPECIAL2, will keep turning right so I can enter SPECIAL2_1R");
        
       }
       break;
      }

      case SPECIAL2_1L: // Keep turning left until the front is cleared. Once the front is cleared, enter state SPECIAL3_L. The object is then to the right
      if(!objectInFront()) {
        currentState = FC_LC_RB;
        USstate = SPECIAL3_L;
        Serial.println("Restrictions: Right blocked. State: Entering SPECIAL3_L");
        break;
      }
      else { // keep turning left
        currentState = FB_RB_TURN_LEFT;
        Serial.println("Restrictions: Front and right blocked. State: SPECIAL2_1L. Staying in this state until front is clear.");
        break;
      }
   

      case SPECIAL2_1R: //Keep turning right until the front is cleared. Once the front is cleared, enter SPECIAL3_R. The object is then to the left
      if(!objectInFront()) {
        currentState = FC_LB_RC;
        USstate = SPECIAL3_R;
        Serial.println("Restritcions: Left blocked. State: Entering SPECIAL3_R");
        break;
      }
      else {
        currentState = FB_LB_TURN_RIGHT;
        Serial.println("Restrictions: Front and left blocked. State: SPECIAL2_1R. Staying in this state until front is clear.");
        break;
      }

      case SPECIAL3_L: //Enter this state once turning left is complete. Then, treat as if right is blocked
      if(!objectInFront()) {
          currentState = FC_LC_RB; //Front cleared, left cleared, right blocked
          if(objectRight()) { //if there is an object to the right, enter Special4_L
          USstate = SPECIAL4_L; //Special4_L: The car has backed up, turned left and the object has been detected to the right
          Serial.println("The car has backud up, turned left and object has been detected to the right. STATE: SPECIAL3_L.");
          break;
        }
      }
        
        

      case SPECIAL3_R: //Enter this state once turning right is complete. Then, treat ass if left is blocked.
      if(!objectInFront()) {
        currentState = FC_LB_RC; //Front cleared, left blocked, right cleared
        if(objectLeft()) { //If object to the left, enter Special4_R
        USstate = SPECIAL4_R; //Special4_R: The car has backed up, turned right and the object has now been detected to the left of the car
        Serial.println("The car har backed up, turned right and object has now been detected on the right. STATE: SPECIAL3_R.");
        break;
      }
      }
     

      case SPECIAL4_L: //The car has backed up, turned left and has an object to the right, we want to go forward until this object is cleared and then go back to normal.
      if(!objectRight()) { //if the object is cleared
        currentState = FREE;
        USstate = NORMAL;
        Serial.println("The are no objects. STATE: Ending special state (SPECIAL4_L) and returning to normal conditions");
        break;
      }
      else {
        USstate = SPECIAL4_L;
        Serial.println("There is still a object to the right, keep going forward until its clear. STATE: stay in SPECIAL4_L");
        break;

      
      case SPECIAL4_R: //The car has backed up, turned right and has an object to the left. We want to clear this object and then go back to normal.
      if(!objectLeft()) {
        currentState = FREE;
        USstate = NORMAL;
         Serial.println("The are no objects. STATE: Ending special state (SPECIAL4_R) and returning to normal conditions");
        break;
      }
      else {
        currentState = FREE;
        USstate = NORMAL;
         Serial.println("There is still a object to the left, keep going forward until its clear. STATE: stay in SPECIAL4_R");
      }

  
}
}


    
}

}
