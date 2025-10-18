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

// Created by Igor Markov, July 1997, VLSICAD ABKGroup UCLA/CS

#include <fstream>
#include <iostream>
#include "abkcommon.h"
#include "placement.h"
#include "rwalk.h"
#include "perturb.h"

Perturbation::
Perturbation(Type type, double maxDeltaX, double maxDeltaY):
       _type(type),_maxDeltaX(maxDeltaX),_maxDeltaY(maxDeltaY),
       _haveSeed(false), xWalk(NULL), yWalk(NULL)
{ }

Perturbation::
Perturbation(Type type, double maxDeltaX, double maxDeltaY, unsigned seedN):
       _type(type),_maxDeltaX(maxDeltaX),_maxDeltaY(maxDeltaY),
       _haveSeed(true), _seed(seedN), xWalk(NULL), yWalk(NULL)
{ }

void Perturbation::_initWalks(unsigned size)
{
   if (xWalk) delete xWalk;
   if (yWalk) delete yWalk;
   if (_haveSeed)
   {
      xWalk=new RandomWalk(size,_maxDeltaX,0.0,_seed);
      yWalk=new RandomWalk(size,_maxDeltaY,0.0,square(xWalk->getSeed()));
   }
   else
   {
      xWalk=new RandomWalk(size,_maxDeltaX,0.0);
      yWalk=new RandomWalk(size,_maxDeltaY,0.0,square(xWalk->getSeed()));
   }
}

Perturbation::~Perturbation()
{
   if (xWalk) delete xWalk;
   if (yWalk) delete yWalk;
}

void Perturbation::perturb(Placement& pl) 
{
   abkfatal(pl.getSize()!=0," Attempting to perturb a Placment of size 0 ");
   _initWalks(pl.getSize());
   switch (_type)
   {
      case _PreserveRanks : _perturbPreservingRanks(pl);
                           break;
      case _PreserveCOG   : _perturbPreservingCOG(pl);
                           break;
      default            : abkfatal(0," Unknown perturbation type ");
   }
}

void Perturbation::_perturbPreservingRanks(Placement& pl) 
{
     unsigned k, size=pl.getSize();
     vector<double> vecX(size), vecY(size);
     for (k=0;k!=size;k++)
     {
         vecX[k]=pl[k].x;
         vecY[k]=pl[k].y;
     }

     Permutation xIdx;
     Permutation yIdx;
     
     Permutation xInv(vecX);
     Permutation yInv(vecY);

     xInv.getInverse(xIdx);
     yInv.getInverse(yIdx);


     double epsilon=1e-4;
     double xLag=0.0, yLag=0.0;
     for(k=1;k!=size;k++)
     {
         double newX=pl[xIdx[k]].x+xWalk->getLoc(k)-xLag;
         double newY=pl[yIdx[k]].y+yWalk->getLoc(k)-yLag;
         if (newX<pl[xIdx[k-1]].x)
         {
            xLag=pl[xIdx[k-1]].x+epsilon-newX;
            newX=pl[xIdx[k-1]].x+epsilon;
         }
         else xLag=0.0;
         if (newY<pl[yIdx[k-1]].y)
         {
            yLag=pl[yIdx[k-1]].y+epsilon-newY;
            newY=pl[yIdx[k-1]].y+epsilon;
         }
         else yLag=0.0;

         pl[xIdx[k]].x=newX;
         pl[yIdx[k]].y=newY;
     }
}

void Perturbation::_perturbPreservingCOG(Placement& pl) 
{
     unsigned size=pl.getSize();
     for(unsigned k=0;k!=size;k++)
     {
        pl[k].x+=xWalk->getStep(k); 
        pl[k].y+=yWalk->getStep(k); 
     }
}
