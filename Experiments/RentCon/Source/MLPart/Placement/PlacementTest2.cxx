/**************************************************************************
***    
*** Copyright (c) 1995-2000 Regents of the University of California,
***               Andrew E. Caldwell, Andrew B. Kahng and Igor L. Markov
*** Copyright (c) 2000-2007 Regents of the University of Michigan,
***               Saurabh N. Adya, Jarrod A. Roy, David A. Papa and
***               Igor L. Markov
***
***  Contact author(s): abk@cs.ucsd.edu, imarkov@umich.edu
***  Original Affiliation:   UCLA, Computer Science Department,
***                          Los Angeles, CA 90095-1596 USA
***
***  Permission is hereby granted, free of charge, to any person obtaining 
***  a copy of this software and associated documentation files (the
***  "Software"), to deal in the Software without restriction, including
***  without limitation 
***  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
***  and/or sell copies of the Software, and to permit persons to whom the 
***  Software is furnished to do so, subject to the following conditions:
***
***  The above copyright notice and this permission notice shall be included
***  in all copies or substantial portions of the Software.
***
*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
*** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
*** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
*** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
*** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
*** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
*** THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***
***
***************************************************************************/


// Created by Igor Markov, July 1997, VLSICAD ABKGroup UCLA/CS
#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif

#include <fstream>
#include "placement.h"
#include "chull.h"

using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
   cout << "Polygon area and convex hull test" << endl;
   Placement pl0(4);
   pl0[0]=Point(0,0);
   pl0[1]=Point(1,0);
   pl0[2]=Point(1,1);
   pl0[3]=Point(0,1);
   CHull ch0(pl0);
   cout << "Area : " << ch0.getPolygonArea() << endl;
   Point pt(0.3,0.5);
   cout << "Point " << pt << "\n is inside: " <<ch0.isInsidePolygon(pt) << endl;
   return 0;

   Param printReq("p",Param::BOOL,argc,argv);
   Param number("n",Param::UNSIGNED,argc,argv);
   abkassert(number.found()," Need -n <number> ");

   Placement pl(Placement::_Grid1,number.getUnsigned());
   if (printReq.found()) cout << pl;

   Timer tm1;
   CHull ch(pl);
   tm1.stop();
   cout << "Convex hull for "<<number.getUnsigned()<<" points took "<<tm1<<endl;
   cout << "Area is " << ch.getPolygonArea() << endl;
   if (printReq.found()) cout << "Convex hull \n" << ch;
   return 0;
}
