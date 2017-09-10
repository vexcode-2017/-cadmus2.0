
#include "main.h"
#include "drive.h"
#include "lift.h"
#include "scoop.h"
#include "claw.h"
#include "ports.h"
#include "autostack.h"
#include "arm.h"

void ptest(int port){
	motorSet(port, 127);
	delay(100);
	motorSet(port, -127);
	delay(100);
}

void operatorControl() {
	while (1) {
		armTest();
		arcade();
		scoop();
		grab();
		arm(joystickGetAnalog(1, 2));
		pclaw();
		autoStack();
		delay(20);

		lcdPrint(uart1, 1, "Potentiometer %d", analogRead(LIFTPOT));
	}
}

/*
while(574C_Members > 0){
	int distance = findNearestTallObject();
	if(distance == walkable){
		walk();
		climb();
		jump();
	}else{
		noose();
	}
}
*/
