#include "main.h"
#include "sensorTargets.h"

void scoop(int vel){
  motorSet(SCOOP1, -vel);
  motorSet(SCOOP2, -vel);
}

void scoopOp(){
  if(buttonGetState(JOY1_5U)){
    scoop(127);
  }else if(buttonGetState(JOY1_6U)){
    scoop(-70);
  }else{
    scoop(0);
  }
}

void autoScoop(int target){
  //0 = down, 1 = up
  if(target == 0){
    scoopTarget = 0;
    scoop(-127);
    while(analogRead(SCOOPPOT) > SP_BOT) delay(20);
  }else if(target == 1){
    scoopTarget = 1;
    scoop(127);
    while(analogRead(SCOOPPOT) < SP_TOP) delay(20);
  }
  if(scoopTarget < 2) scoop(0);
}

void manualDrop(){
  scoopTarget = 2;
  scoop(127);
  delay(1000);
  scoop(0);
}
