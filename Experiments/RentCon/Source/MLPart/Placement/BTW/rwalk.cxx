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

// 971113 mro removed unseeded ctor for RandomWalk (use default value)

#include "abkcommon.h"
#include "rwalk.h"

 
RandomWalk::
RandomWalk(unsigned size,double maxStep,double endsAt,unsigned seedN):
           _locations(size),_steps(size),_maxStep(maxStep),_endsAt(endsAt),
           _randouble(0,1,seedN)
{
   abkfatal(size!=0,"Can't create a RandomWalk with 0 steps");
   abkfatal(abs(_endsAt)/size<=_maxStep,
       " Impossible random walk : step too small ");
   _generate();
}
void RandomWalk::_generate()
{
   double sum=0.0;
   unsigned k=0, size=_steps.size();
   while (k<size-1)  // note: k can decrease
   {
      double maxReserve= _maxStep*(size-k-1) - (sum - _endsAt) ;
      double minReserve=-_maxStep*(size-k-1) - (sum - _endsAt) ;
      if (maxReserve < -_maxStep || minReserve > _maxStep ) 
      {
         abkfatal(k!=0,
         " Random Walk failed to initialize, email to imarkov@cs.ucla.edu ");
         k--;
         continue;
      }
      double hi =min(maxReserve,_maxStep);
      double lo =max(minReserve,-_maxStep);

      _locations[k++] = sum += _steps[k] = lo+_randouble*(hi-lo);
   }

   if (sum - _endsAt < -_maxStep || sum - _endsAt > _maxStep)
      abkfatal(0,
         " Random Walk failed to initialize, email to imarkov@cs.ucla.edu ");

   _locations[k] = sum += _steps[k] = - (sum - _endsAt) ;   
}


ostream& operator<<(ostream& out,const RandomWalk& rwk)
{
   unsigned size=rwk.getSize();
   out << " Random walk of length " << size << endl;
   for (unsigned k=0;k!=size;k++)
       out << setw(10) << rwk.getStep(k) << setw(10) << rwk.getLoc(k) << "\n";
   return out;
}
