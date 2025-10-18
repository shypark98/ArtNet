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

// Created by Igor Markov, September 1997, VLSICAD ABKGroup UCLA/CS

#include <fstream>
#include <iostream>
using std::cout;
using std::endl;
#include "transf.h"
#include "plStats.h"

int main()
 {
  // Placement pl(Placement::_Grid1,3);
  Placement pl(Placement::_Random,100);
  BBoxToBBox(BBox(0,0,1,1),BBox(2,2,10,10))(pl);
//ofstream("orig.pla") << pl;
  {
      Placement pl1(pl);
      ClumpInCenter clump(pl1.getBBox(),Point(5,5),3.0);    // alpha==3.0
      clump(pl1);
      cout << "Clumping points in pl near the center -- producing pl1" << endl;
      cout << "PlCorr(pl,pl1)      = " << PlCorr(pl,pl1) << endl;
      cout << "PlRankCorr(pl,pl1)  = " << PlRankCorr(pl,pl1) << endl;
//    ofstream("clumped.pla") << pl1;
  }
  {   
      Placement pl2(pl);
      ClumpInCenter unclump(pl2.getBBox(),Point(5,5),1.0/3.0); // alpha==1/3.0
      unclump(pl2);
      cout << "Unclumping points in pl from the center -- producing pl2"<< endl;
      cout << "PlCorr(pl,pl2)      = " << PlCorr(pl,pl2) << endl;
      cout << "PlRankCorr(pl,pl2)  = " << PlRankCorr(pl,pl2) << endl;
//    ofstream("unclumped.pla") << pl2;
  }
}
