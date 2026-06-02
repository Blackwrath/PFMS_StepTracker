/*
 * StepIdentifier.c
 *
 *  Created on: May 4, 2026
 *      Author: willa
 */
#include <stdint.h>
#include <stdbool.h>
#include "StepIdentifier.h"
#include "stm32c0xx_hal.h"
int32_t timeCurrentStep;

Vector3 vecSub(Vector3 v1, Vector3 v2){
	Vector3 out;
	out.x = v1.x-v2.x;
	out.y = v1.y-v2.y;
	out.z = v1.z-v2.z;
	return out;
}
Vector3 scalarMult(int s, Vector3 v1){
	Vector3 out;
	out.x = v1.x*s;
	out.y = v1.y*s;
	out.z = v1.z*s;
	return out;
}
int dotProd(Vector3 v1, Vector3 v2){
	int out = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
	return out;
}
int magSquared(Vector3 v1){
	int out = v1.x*v1.x+v1.y*v1.y+v1.z*v1.z;
	return out;
}



void TrackStep(Vector3 currentVector, Vector3 calibrationVector, bool* Step,int* stepCount){
	int32_t timeNow = HAL_GetTick();
	int32_t timeBetweenSteps = timeNow - timeCurrentStep;
	int threshhold = 200;
	// Assume the calibration vector is the direction gravity is pointing.
	int calibrationMag = magSquared(calibrationVector);
	Vector3 CurrentVectorCalibrated = vecSub(currentVector,calibrationVector); // This is the current acceleration taking into account gravity
	//int dir = dotProd(CurrentVectorCalibrated,calibrationVector); // How much of the current vector is pointing along the calibrated gravity vector?
	////Vector3 MagnitudeVector = scalarMult(dir,calibrationVector); // Projection of the calibrated vector onto the calibration vector
	int finalMag = magSquared(CurrentVectorCalibrated);
	if (finalMag>threshhold*threshhold && timeBetweenSteps > 30){// If we're above the threshhold
		if(!*Step){ // If we're not already in a step
			timeCurrentStep = timeBetweenSteps;
			*stepCount +=1; //increment the step count
			*Step = true; //We are now in a step
		}// If we're above the threshhold, but currently in a step do nothing
	}else{// If we're below the threshhold
		if(*Step){
			*Step = false; //We are now below the threshhold, so we are no longer in a step
		}
	}

}
