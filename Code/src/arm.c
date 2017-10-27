#include "main.h"
#include "ports.h"



//reset the encoder if the arm bottoms out
void armTest(){
  if(digitalRead(ARM_LIMIT) == LOW){
    encoderReset(armEnc);
  }
}

// motion control for the arm
void arm(int vel){
  motorSet(ARM, vel);
}

//arm PID control (basically copy pasted from liftPID)
void armPID(int sp){
  //define coefficients
  double kp = 5;
  double ki = 0.2;
  double kd = 3;

  // define local variables
  int speed; // speed
  int derivative; // derivative
  int integral = 0;
  int prevErr = 0;

  int sv = encoderGet(armEnc); // get sensor value
  int error = sp - sv; // find error
  integral = integral + error; // calculate integral

  derivative = error - prevErr; // calculate the derivative
  prevErr = error; // set current error to equal previous error

  if(error > 10){integral = 0;} // only modify integral if close to target

  speed = error*kp + integral*ki + derivative*kd;; // add the values to get the motor speed

  arm(speed); // set the lift to the speed
}
