#pragma once
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "defines.h"
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define debug 0   //Enable this macro to start debugging

#define velLOG    "log/velocity.log"
#define divLOG    "log/divergence.log"
#define pressLOG  "log/pressure.log"
#define colorLOG  "log/color.log"
#define vortLOG   "log/vorticity.log"

static FILE *vel, *diver, *press, *color, *vort ;

using namespace std;

void openLOG();

void displayVelocity(float velocity[][DIM][2]);

void displayDivergence(float divergence[][DIM]) ;

void displayPressure(float pressure[][DIM]);

void displayColor(float dye[][DIM][4]);

void displayVorticity(float vorticity[][DIM]);

void printDetails();

#endif
