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

#include "subPlacement.h"

using std::cout;
using std::endl;

int main()
{
   cout << " Sub placement test" << endl;
   Placement pl(4),smallP(1);
   pl[0]=Point(0,0);
   pl[1]=Point(1,0);
   pl[2]=Point(1,1);
   pl[3]=Point(0,1);
   Mapping mp(3,4);
   mp[0]=0; mp[1]=2, mp[2]=3;
   SubPlacement ppl(pl,mp);
 //Point pt(0.3,0.5);

   cout << ppl;
   ppl.pullBack(smallP);
   cout << smallP; 
   smallP[1]=Point(2,2);
   ppl.pushForward(smallP);
   cout << pl;
   cout << "Area : " << pl.getPolygonArea() << endl;
   return 0;
}
