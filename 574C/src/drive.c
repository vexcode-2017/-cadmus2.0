#include "main.h"
#include "math.h"

//generic drive functions =============================================
void leftD(int vel){
  motorSet(DRIVEL1, vel);
  motorSet(DRIVEL2, vel);
}
void rightD(int vel){
  motorSet(DRIVER1, vel);
  motorSet(DRIVER2, vel);
}
void drive(int vel){
  rightD(-vel);
  leftD(-vel);
}
void slant(int vel, int right){
  drive(127);
  if(right){
    leftD(-vel);
  }else{
    rightD(-vel);
  }
}

// power curve =============================================
void tankSigLPC(){
  double left;
  double right;
  double rJoy = -joystickGetAnalog(1, 3);
  double lJoy = -joystickGetAnalog(1, 2);
  int deadzone = 8;

  if(abs(lJoy) > deadzone){
    left = log(256/(lJoy+128)-1)/(-.025);
  }else{
    left = 0;
  }
  if(abs(rJoy) > deadzone){
    right = log(256/(rJoy+128)-1)/(-.025);
  }else{
    right = 0;
  }

  leftD(left);
  rightD(right);
}

// autonomous drive functions =============================================
//drive until (for breaking up long drives into segments)
void driveUntil(int sp){
  encoderReset(driveEncLeft);
  encoderReset(driveEncRight);

  int dir = 1; //forward
  if(sp < 0) dir = 0; //backward
  int speed;
  if(dir == 1) speed = 127;
  if(dir == 0) speed = -127;
  while(1){
    int sv = (encoderGet(driveEncLeft)+encoderGet(driveEncRight))/2;
    int error = sp-sv;
    drive(speed);
    if(dir == 0 && error > 0) break;
    if(dir == 1 && error < 0) break;
    delay(20);
  }
}

// forward and backward
void autoDrive(int sp){
  encoderReset(driveEncLeft);
  encoderReset(driveEncRight);

  int prevError = 0;

  //PD variables
  double kp = 0.3;
  double kd = 0.6;

  //kc control
  int kc = -.04*abs(sp) + 64;
  if(kc < 20) kc = 20;
  if(hasPylon) kc += 15;

  //kb scaling for battery level
  double kb = powerLevelMain()*-0.0118+100;

  int brake = 50;
  int dir = 1; //forward
  if(sp < 0){ //backward
    dir = 0;
    brake = -brake;
    kb = -kb;
  }

  while(1){
    int sv = (encoderGet(driveEncLeft)+encoderGet(driveEncRight))/2;
    int error = sp-sv;
    int derivative = error-prevError;
    int speed = error*kp + derivative*kd + kb;
    prevError = error;

    //speed normalization
    if(dir == 1 && speed < kc) speed = kc;
    if(dir == 0 && speed > -kc) speed = -kc;

    drive(speed);

    if(dir == 0 && error > 0) break;
    if(dir == 1 && error < 0) break;
    lcdPrint(uart1, 1, "encAvg: %d", sv);
    delay(20);
  }
  drive(-brake);
  delay(80);
  drive(0); // stop drive
}


void sonarDrive(){
  drive(127); // start driving forward
  int u; // initialize the container for gyro
  do{
    u = ultrasonicGet(sonar);
    delay(10);
  }while(u > 6 || u == 0); // detect pylon in a certain range
  drive(0);
}

void sonarDriveDistance(int sp){
  double kp = 0.25;
  int kc = 45;
  int brake = 50;
  int u; // initialize the container for gyro
  do{
    int sv = (encoderGet(driveEncLeft)+encoderGet(driveEncRight))/2;
    int error = sp-sv;
    int speed = error*kp;
    if(speed < 0) speed = 0;
    if(speed < kc) speed = kc;
    drive(speed);
    u = ultrasonicGet(sonar);
    delay(10);
  }while(u > 6 || u == 0); // detect pylon in a certain range
  delay(200);
  drive(-brake);
  delay(50);
  drive(0); // stop drive
}


void gyTurn(int sp){
  int redux = 4;
  int error = sp - gyRead();

  if(error > 0) sp -= redux;
  if(error < 0) sp += redux;
  if(autoRight == true){
    sp = -sp; // inverted turn speed for right auton
  }
  int prevError = 0;

  double kp = 2.4;
  double kd = 12;

  int brake = 40;

  //kc control
  int kc = -.25*abs(error) + 62;
  if(kc < 30) kc = 30;
  if(hasPylon == true) kc += 15;

  //kb scaling for battery level
  double kb = powerLevelMain()*-0.0118+100;

  //set direction
  int dir = 0; //left
  if(error < 0){
    kb = -kb;
    dir = 1;
    brake = -brake;
  }


  while(1){
    int sv = gyRead(); // get sensor

    //calculate speed
    error = sp-sv;
    int derivative = error-prevError;
    int speed = error*kp + derivative*kd;
    prevError = error;


    //velocity normalization
    if(speed > 127) speed = 127;
    if(speed < -127) speed = -127;

    //kc enforcement
    if(dir == 0 && speed < kc) speed = kc;
    if(dir == 1 && speed > -kc) speed = -kc;

    //end loop check
    if(dir == 0 && error <= 0) break;
    if(dir == 1 && error >= 0) break;

    leftD(-speed);
    rightD(speed);

    lcdPrint(uart1, 1, "gyro: %d", sv);
    delay(20);
  }
  leftD(brake);
  rightD(-brake);
  delay(45);
  drive(0); // stop drive
}

void gyAlign(int sp){
  int integral = 0;
  int kc = 30;
  int kp = 1;
  int ki = 1.8;

  if(hasPylon == true) kc += 15;

  //set direction
  int dir = 0; //left
  if(sp - gyRead() < 0) dir = 1;

  while(1){
    int sv = gyRead(); // get sensor

    //calculate speed
    int error = sp-sv;
    integral = integral + error;
    int speed = error*kp + integral*ki;

    //kc enforcement
    if(dir == 0 && speed < kc) speed = kc;
    if(dir == 1 && speed > -kc) speed = -kc;

    //end loop check
    if(dir == 0 && error <= 0) break;
    if(dir == 1 && error >= 0) break;

    leftD(-speed);
    rightD(speed);

    lcdPrint(uart1, 1, "gyro: %d", sv);
    delay(20);
  }
  drive(0); // stop drive
}
