/*
 * StepIdentifier.h
 *
 *  Created on: May 4, 2026
 *      Author: willa
 */

#ifndef APPLICATION_USER_LIB_STEPIDENTIFIER_H_
#define APPLICATION_USER_LIB_STEPIDENTIFIER_H_
#include <stdbool.h>

typedef struct { // Imma need to do some vector maths, so I'm making my own collection of functions for working with Vector3
	int x;
	int y;
	int z;
} Vector3;

Vector3 vecSub(Vector3 v1, Vector3 v2);
Vector3 scalarMult(int s, Vector3 v1);
int dotProd(Vector3 v1, Vector3 v2);
int magSquared(Vector3 v1);
void TrackStep(Vector3 currentVector, Vector3 calibrationVector, bool* Step,int* stepCount);

#endif /* APPLICATION_USER_LIB_STEPIDENTIFIER_H_ */
