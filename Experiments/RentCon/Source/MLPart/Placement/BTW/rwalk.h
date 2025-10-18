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

// Created:    Igor Markov,  VLSI CAD ABKGROUP UCLA  Sept 29 1997

// 971113 mro unified seeded, unseeded ctors in RandomWalk

#ifndef _RWALK_H_
#define _RWALK_H_

#include "abkcommon.h"
 
class RandomWalk;
typedef RandomWalk RWalk;

class RandomWalk 
// One-dimensional fixed length random walk starting at 0.0 
// and returning to a predefined location (default 0.0)
// with steps limited by _maxStep in each direction
//
// if needed, I can increase the quality of "randomness" by an additional
// random shuffle of random steps
{
   double            _maxStep;
   double            _endsAt;
   vector<double>    _locations;
   vector<double>    _steps;
   RandomDouble      _randouble;

   void _generate();
public:
   RandomWalk(unsigned size,double maxStep,
              double endsAt=0.0, unsigned seedN=UINT_MAX);

   // to "reconstruct events"
   unsigned getSeed()           const { return _randouble.getSeed(); }

   // synonyms
   unsigned getSize()           const { return _steps.size();   }
   unsigned getLength()         const { return _steps.size();   }

   double getStep(unsigned idx) const { return _steps[idx];     }
   double getLoc(unsigned idx)  const { return _locations[idx]; }

   const vector<double>& getSteps() const { return _steps;      }
   const vector<double>& getLocs () const { return _locations;  }
};

ostream& operator<<(ostream&,const RandomWalk&);

#endif
