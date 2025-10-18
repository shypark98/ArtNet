/**************************************************************************
***
***    Copyright (c) 1995-1997 by ABKGroup, UCLA VLSI CAD Laboratory,
***    UCLA Computer Science Department, Los Angeles, CA 90095-1596 USA,
***    and by the Regents of the University of California.
***    All rights reserved.
***
***    No portion of this file may be used, copied, or transmitted for
***    any purpose, including any research or commercial purpose,
***    without the express written consent of Professor Andrew B. Kahng,
***    UCLA Computer Science Department, 3713 Boelter Hall, Los Angeles,
***    CA 90095-1596 USA.   Any copy made of this file must include this
***    notice.    ABKGroup and the Regents of the University of California
***    retain title to all copies of this file and/or its compiled
***    derivatives.
***
***************************************************************************/

// Created by Igor Markov, Sept 29 1997, VLSICAD ABKGroup UCLA/CS

#ifndef _PERTURB_H_
#define _PERTURB_H_

#include <fstream>
#include <iostream>
#include "abkcommon.h"
#include "placement.h"
#include "rwalk.h"

class Perturbation
{
public:
   // _PreserveRanks randomly shifts cells relative to each other 
   // _PreserveCOG   shifts cells independently
   // _PreserveRanks preserves COG too
   enum Type { _PreserveRanks, _PreserveCOG }; 
private:
   Type   _type;
   double _maxDeltaX;
   double _maxDeltaY;
   bool      _haveSeed;
   unsigned  _seed;

   RandomWalk *xWalk;
   RandomWalk *yWalk;

   void _initWalks(unsigned size);

   void _perturbPreservingRanks(Placement&);
   void _perturbPreservingCOG  (Placement&);

public:
   Perturbation(Type type, double maxDeltaX, double maxDeltaY);
   Perturbation(Type type, double maxDeltaX, double maxDeltaY, unsigned seedN);
   ~Perturbation();
   void perturb(Placement&);
 
   unsigned getSeed() const { return xWalk->getSeed(); }
};

#endif
